def create_large_list(size):
    large_list = []
    for i in range(size):
        large_list.append(" " * (1024 * 1024))  # Appending 1 MB of empty strings to the list
    return large_list
# Creating a list with 10,000 empty strings, each of size 1 MB
my_large_list = create_large_list(10000)
# Using a lot of memory for a relatively simple data structure