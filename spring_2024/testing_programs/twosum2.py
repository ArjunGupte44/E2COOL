def two_sum(nums, target):
    n = len(nums)
    num_map = {}  # Use a hash map to store the array elements
    for i in range(n):
        num_map[nums[i]] = i  # Add the element and its index to the map
    for i in range(n):
        if (target - nums[i]) in num_map:  # Check if the complement exists in the map
            return [i, num_map[target - nums[i]]]  # Return the indices of the two numbers
    return None
def main():
    # Generate a large test case
    nums = list(range(1, 5001))  # Array of size 5000 with consecutive integers
    target = 9999  # Choose a target sum
    # Run the Two Sum method
    result = two_sum(nums, target)
    if result:
        print(f"Indices with the sum {target}: {result}")
    else:
        print("No solution found.")
if __name__ == "__main__":
    main()