# Rewritten Code:
# This function performs matrix multiplication.
# It multiplies two matrices 'first' and 'second', storing the result in 'result'.
def multiply_matrices(first, second, result, r1, c1, r2, c2):
    # Iterate through rows of the first matrix
    for i in range(r1):
        # Iterate through columns of the second matrix
        for j in range(c2):
            # Perform element-wise multiplication
            prod = 1
            for k in range(c1):
                # Multiply current element from first and second matrices
                prod *= first[i][k] * second[k][j]
            # Accumulate the product in the result matrix
            result[i][j] += prod

# Main function to set up matrices and perform multiple multiplications
def main(num_mult, mat_dim):
    # Initialize the first matrix with a specific pattern
    first = [[i * j * 0.5 for j in range(mat_dim)] for i in range(mat_dim)]
    # Initialize the second matrix with a different pattern
    second = [[(i + j) * 0.5 for j in range(mat_dim)] for i in range(mat_dim)]
    # Initialize the result matrix with zeros
    result = [[0 for _ in range(mat_dim)] for _ in range(mat_dim)]

    # Perform matrix multiplication 'num_mult' times
    for _ in range(num_mult):
        multiply_matrices(first, second, result, mat_dim, mat_dim, mat_dim, mat_dim)

    # Print a specific element of the result matrix as a sample output
    print("Result[5][5]:", result[5][5])

# Entry point of the script
if __name__ == "__main__":
    import sys
    # Set the number of multiplications to perform
    num_mult = int(100)
    # Set the dimension of the square matrices
    mat_dim = int(100)
    # Call the main function with these parameters
    main(num_mult, mat_dim)