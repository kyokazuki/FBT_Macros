import sys, os

file_dir = str(sys.argv[1])

highest_num = 0
for file in os.listdir(file_dir):
    if not os.path.isfile(os.path.join(file_dir, file)):
        continue
    
    if not file[0:4].isdigit():
        continue

    file_num = int(file[0:4])
    if file_num > highest_num:
        highest_num = file_num

print(f"{highest_num+1:04d}")
