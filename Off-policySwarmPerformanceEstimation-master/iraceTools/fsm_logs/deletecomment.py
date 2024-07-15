import re

def remove_multiline_comments_v3(file_path, output_path):
    with open(file_path, 'r') as file:
        content = file.read()
    
    # Regular expression to match multi-line comments
    pattern = re.compile(r'("""[\s\S]*?"""|\'\'\'[\s\S]*?\'\'\')', re.DOTALL)
    filtered_content = re.sub(pattern, '', content)
    
    # Remove empty lines
    filtered_content = '\n'.join([line for line in filtered_content.split('\n') if line.strip() != ''])
    
    with open(output_path, 'w') as file:
        file.write(filtered_content)


# Execute the function
output_path = '/mnt/data/filtered_output_v3.py'
remove_multiline_comments_v3('6_fsm_log.py', '6_fsm_log_no_comments.txt')