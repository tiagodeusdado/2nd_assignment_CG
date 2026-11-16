import struct
import json


def multiply_mat4(A, B):
    # A and B are 16-element lists (column-major)
    # C = A * B
    C = [0.0] * 16
    for col in range(4):
        for row in range(4):
            val = 0.0
            for k in range(4):
                val += A[k * 4 + row] * B[col * 4 + k]
            C[col * 4 + row] = val
    return C

def identity_mat4():
    return [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    ]

def transform_vec3(M, v):
    # M is 16-element column-major
    x = M[0] * v[0] + M[4] * v[1] + M[8] * v[2] + M[12]
    y = M[1] * v[0] + M[5] * v[1] + M[9] * v[2] + M[13]
    z = M[2] * v[0] + M[6] * v[1] + M[10] * v[2] + M[14]
    w = M[3] * v[0] + M[7] * v[1] + M[11] * v[2] + M[15]
    if abs(w) > 1e-7 and abs(w - 1.0) > 1e-7:
        x /= w
        y /= w
        z /= w
    return [x, y, z]

def transform_normal(M, n):
    # For normals, strictly inverse-transpose, but here scale/rotation:
    x = M[0] * n[0] + M[4] * n[1] + M[8] * n[2]
    y = M[1] * n[0] + M[5] * n[1] + M[9] * n[2]
    z = M[2] * n[0] + M[6] * n[1] + M[10] * n[2]
    l = (x*x + y*y + z*z)**0.5
    if l > 1e-8:
        x /= l
        y /= l
        z /= l
    return [x, y, z]

def node_to_matrix(node):
    if 'matrix' in node:
        return list(node['matrix'])
    M = identity_mat4()
    # We can handle translation, rotation, scale if present
    # Currently in our runway glb, all nodes with transform have 'matrix'
    return M

with open('runway.glb', 'rb') as f:
    magic, ver, length = struct.unpack('<4sII', f.read(12))
    chunk_len, chunk_type = struct.unpack('<I4s', f.read(8))
    gltf = json.loads(f.read(chunk_len).decode('utf-8'))
    chunk_bin_len, chunk_bin_type = struct.unpack('<I4s', f.read(8))
    bin_data = f.read(chunk_bin_len)

def get_accessor_data(acc_idx):
    acc = gltf['accessors'][acc_idx]
    bv = gltf['bufferViews'][acc['bufferView']]
    offset = bv.get('byteOffset', 0) + acc.get('byteOffset', 0)
    count = acc['count']
    comp_type = acc['componentType']
    type_str = acc['type']
    
    type_size = {
        'SCALAR': 1,
        'VEC2': 2,
        'VEC3': 3,
        'VEC4': 4,
        'MAT4': 16
    }[type_str]
    
    format_map = {
        5120: 'b',
        5121: 'B',
        5122: 'h',
        5123: 'H',
        5125: 'I',
        5126: 'f',
    }
    fmt_char = format_map[comp_type]
    stride = bv.get('byteStride', struct.calcsize('<' + fmt_char * type_size))
    
    result = []
    curr = offset
    for _ in range(count):
        vals = struct.unpack_from('<' + fmt_char * type_size, bin_data, curr)
        if type_size == 1:
            result.append(vals[0])
        else:
            result.append(list(vals))
        curr += stride
    return result

# Compute global matrix for each node
nodes = gltf['nodes']
global_matrices = [None] * len(nodes)

def compute_node_matrix(node_idx, parent_mat):
    node = nodes[node_idx]
    local_mat = node_to_matrix(node)
    world_mat = multiply_mat4(parent_mat, local_mat)
    global_matrices[node_idx] = world_mat
    for child in node.get('children', []):
        compute_node_matrix(child, world_mat)

# Find root nodes (nodes not referenced in any children)
all_children = set()
for n in nodes:
    for c in n.get('children', []):
        all_children.add(c)

roots = [i for i in range(len(nodes)) if i not in all_children]
for r in roots:
    compute_node_matrix(r, identity_mat4())

print(f"Processed {len(nodes)} nodes.")

# Now gather all meshes and primitives and write to runway.obj
total_v = 0
total_vn = 0
total_f = 0

all_vertices = []
all_normals = []
all_faces = []

min_bounds = [float('inf')]*3
max_bounds = [float('-inf')]*3

for node_idx, node in enumerate(nodes):
    mesh_idx = node.get('mesh')
    if mesh_idx is None:
        continue
    mat = global_matrices[node_idx]
    mesh = gltf['meshes'][mesh_idx]
    for prim in mesh['primitives']:
        pos_list = get_accessor_data(prim['attributes']['POSITION'])
        norm_list = get_accessor_data(prim['attributes']['NORMAL']) if 'NORMAL' in prim['attributes'] else None
        
        idx_accessor = prim.get('indices')
        if idx_accessor is not None:
            indices = get_accessor_data(idx_accessor)
        else:
            indices = list(range(len(pos_list)))
            
        start_v_idx = total_v + 1 # 1-based obj
        start_vn_idx = total_vn + 1
        
        for p in pos_list:
            tp = transform_vec3(mat, p)
            all_vertices.append(tp)
            for d in range(3):
                min_bounds[d] = min(min_bounds[d], tp[d])
                max_bounds[d] = max(max_bounds[d], tp[d])
        total_v += len(pos_list)
        
        if norm_list:
            for n in norm_list:
                tn = transform_normal(mat, n)
                all_normals.append(tn)
            total_vn += len(norm_list)
            
        for i in range(0, len(indices), 3):
            i0 = indices[i]
            i1 = indices[i+1]
            i2 = indices[i+2]
            if norm_list:
                all_faces.append(f"{start_v_idx + i0}//{start_vn_idx + i0} {start_v_idx + i1}//{start_vn_idx + i1} {start_v_idx + i2}//{start_vn_idx + i2}")
            else:
                all_faces.append(f"{start_v_idx + i0} {start_v_idx + i1} {start_v_idx + i2}")
        total_f += len(indices) // 3

print(f"Writing runway.obj: vertices={total_v}, normals={total_vn}, faces={total_f}")
print(f"Bounds min={min_bounds}, max={max_bounds}")
print(f"Size={[max_bounds[i] - min_bounds[i] for i in range(3)]}")

with open('runway.obj', 'w', encoding='utf-8') as f:
    f.write("# Exported Runway OBJ\n")
    for v in all_vertices:
        f.write(f"v {v[0]:.4f} {v[1]:.4f} {v[2]:.4f}\n")
    for n in all_normals:
        f.write(f"vn {n[0]:.4f} {n[1]:.4f} {n[2]:.4f}\n")
    for face in all_faces:
        f.write(f"f {face}\n")

print("runway.obj successfully created!")
