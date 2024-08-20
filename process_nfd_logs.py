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

print("Processing NFD logs!\n")

output_file_path = 'log.txt'


# Count interest packets coming from application face
keyword1 = 'CABEEE'
keyword2 = 'onIncomingInterestFromApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"\nNumber of generated interest packets by applications (onIncomingInterest to each NFD node from applications): {count} interests\n")
# Count data packets going to application face
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingDataToApp'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"Number of received data packets by applications (onOutgoingData from each NFD node to applications): {count} data\n")


# we can't trust the transmitted packets in ndnSIM because it uses an older version of NFD, which seems to end up with a higher count of transmitted packets than the real NFD
# Count interest packets coming from any face
keyword1 = 'CABEEE'
keyword2 = 'onIncomingInterestFromFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"\nNumber of interest packets into NFD from a physical face (onIncomingInterest to each NFD node from a physical face): {count} interests\n")
# Count data packets going to any face
keyword1 = 'CABEEE'
keyword2 = 'onOutgoingDataToFace'
count = count_specific_lines(output_file_path, keyword1, keyword2)
print(f"Number of data packets leaving NFD on a physical face (onOutgoingData from each NFD node to a physical face): {count} data\n")


