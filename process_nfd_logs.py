import os


def count_specific_lines(output_file_path, keyword1, keyword2):
    count = 0
    with open(output_file_path, 'r', encoding='utf-8') as file:
        for line in file:
            if keyword1 in line and keyword2 in line:
                count += 1
            if "serviceOrchestration/reset" in line:
                count = 0
    return count



#TODO: Delete any leading spaces to make sure all entries have timestamp at the beginning of the line!
#TODO: Add node name after the timestamp when merging!

print("Processing NFD logs!")

output_file_path = 'scenario.log'

print("")

# Count interest packets coming from application face
keyword1 = 'CABEEE'
keyword2 = 'onIncomingWFInterestFromApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"Interest Packets Generated: {count} interests")
# Count data packets going to application face
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingWFDataToApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"Data Packets Generated: {count} data")

print("")

# we can't trust the transmitted packets in ndnSIM because it uses an older version of NFD, which seems to end up with a higher count of transmitted packets than the real NFD
# Count interest packets coming from any face
keyword1 = 'CABEEE'
keyword2 = 'onIncomingWFInterestFromFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"Interest Packets Transmitted: {count} interests")
# Count data packets going to any face
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingWFDataToFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"Data Packets Transmitted: {count} data")

print("")
