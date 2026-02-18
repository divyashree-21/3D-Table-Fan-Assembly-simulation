-- Generated SQL for table_fan_db
CREATE DATABASE IF NOT EXISTS table_fan_db;
USE table_fan_db;

CREATE TABLE IF NOT EXISTS parts (
  id INT AUTO_INCREMENT PRIMARY KEY,
  name VARCHAR(100),
  assembly_order INT,
  disassembly_order INT
);

CREATE TABLE IF NOT EXISTS assembly_summary (
  id INT AUTO_INCREMENT PRIMARY KEY,
  joined_parts TEXT
);

INSERT INTO parts (name, assembly_order) VALUES ('Base', 1);
INSERT INTO parts (name, assembly_order) VALUES ('Stand', 2);
INSERT INTO parts (name, assembly_order) VALUES ('Motor', 3);
INSERT INTO parts (name, assembly_order) VALUES ('Blades', 4);
INSERT INTO parts (name, assembly_order) VALUES ('Front Cover', 5);
INSERT INTO assembly_summary (joined_parts) VALUES ('Base, Stand, Motor, Blades, Front Cover');

UPDATE parts SET disassembly_order = 1 WHERE name = 'Front Cover' AND disassembly_order IS NULL;
UPDATE parts SET disassembly_order = 2 WHERE name = 'Blades' AND disassembly_order IS NULL;
UPDATE parts SET disassembly_order = 3 WHERE name = 'Motor' AND disassembly_order IS NULL;
UPDATE parts SET disassembly_order = 4 WHERE name = 'Stand' AND disassembly_order IS NULL;
UPDATE parts SET disassembly_order = 5 WHERE name = 'Base' AND disassembly_order IS NULL;

-- End of generated SQL
