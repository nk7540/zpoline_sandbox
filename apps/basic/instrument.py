def instrument_c_code(file_name):
    with open(file_name, "r") as f:
        lines = f.readlines()

    new_lines = []

    for line in lines:
        new_lines.append(line)
        if "// event:" in line:
            event_name = line.split("// event:")[1].strip()
            new_lines.append(f'    generate_event("{event_name}");\n')
        if "// assert:" in line:
            file_name = line.split("// assert:")[1].strip()
            new_lines.append(f'    assert(accepts_sequence("{file_name}"));\n')

    with open("instrumented.c", "w") as f:
        f.writelines(new_lines)


instrument_c_code("sample.c")
