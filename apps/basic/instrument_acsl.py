import re

def extract_acsl(policy):
    # Split the policy into individual statements
    statements = policy.strip().split('),(')
    acsl_comments = {}

    for statement in statements:
        parts = statement.replace('(', '').replace(')', '').split(',')
        effect = parts[0].strip()
        principal = parts[1].split(':')[1].strip()
        action = parts[2].split(':')[1].strip()
        resource = parts[3].split(':')[1].strip().replace('(', '').replace(')', '')
        acsl_comment = f"/*@ ensures {effect}({principal}, {action}, {resource}); */"
        acsl_comments[action] = acsl_comment

    return acsl_comments

def instrument_c_program(c_program, policies):
    acsl_comments = extract_acsl(policies)
    
    # For each action in policies, add the corresponding ACSL comment above the function
    for action_name, comment in acsl_comments.items():
        c_program = re.sub(rf'(^[\s]*)([a-zA-Z_]\w*\s+{action_name}\()', rf'\1{comment}\n\1\2', c_program, flags=re.MULTILINE)

    return c_program

if __name__ == '__main__':
    # Read the C program file
    with open('sample.c', 'r') as f:
        c_program = f.read()

    policy = """(( allow,
principal : students,
action : safe_open,
resource : cs240/Exam.pdf),
( allow,
principal : tas,
action : safe_open,
resource : (cs240/Exam.pdf,
cs240/Answer.pdf)))"""

    instrumented_program = instrument_c_program(c_program, policy)

    # Write the instrumented C program to a new file
    with open('sample_with_acsl.c', 'w') as f:
        f.write(instrumented_program)
