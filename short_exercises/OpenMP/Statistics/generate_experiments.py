import random

def generate_large_sample_file(rows, cols, filename="input_bigger.txt"):
    """
    Generates a text file containing a matrix of random integers.

    The format of the file is:
    - First line: number of rows and columns, separated by a space.
    - Subsequent lines: matrix elements, with columns separated by spaces
      and rows separated by newlines.

    Args:
        rows (int): The number of rows for the matrix (samples per experiment).
        cols (int): The number of columns for the matrix (number of experiments).
        filename (str): The name of the output file.
    """
    print(f"Generating file '{filename}' with {rows} rows and {cols} columns...")

    try:
        with open(filename, 'w') as f:
            # Write the header line with dimensions 
            f.write(f"{rows} {cols}\n")

            # Write each row of the matrix
            for _ in range(rows):
                # Generate a list of random integers for the row 
                # The map function converts each integer to a string
                row_data = map(str, [random.randint(0, 99) for _ in range(cols)])
                
                # Join the numbers with a space and write to the file,
                # followed by a newline 
                f.write(" ".join(row_data) + "\n")
        
        print("File generated successfully!")

    except IOError as e:
        print(f"Error writing to file: {e}")


if __name__ == '__main__':
    # --- Configuration ---
    # You can change these values to generate a file of a different size
    NUM_ROWS = 10000 # Represents 'n' samples 
    NUM_COLS = 5000  # Represents 'E' experiments 
    
    generate_large_sample_file(NUM_ROWS, NUM_COLS)