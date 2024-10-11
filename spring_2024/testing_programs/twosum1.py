def two_sum(nums, target):
    n = len(nums)
    for i in range(n):
        for j in range(i + 1, n):
            if nums[i] + nums[j] == target:
                return [i, j]
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