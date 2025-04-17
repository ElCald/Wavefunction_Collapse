import re

def parse_image_times(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    time_pattern = re.compile(r'en\s*:\s*([\d.]+) sec')
    execution_times = []

    for line in lines:
        match = time_pattern.search(line)
        if match:
            execution_time = float(match.group(1))
            execution_times.append(execution_time)

    if execution_times:
        average_time = sum(execution_times) / len(execution_times)
        print(f"Nombre de mesures : {len(execution_times)}")
        print(f"Moyenne des temps d'exécution = {average_time:.6f} secondes")
    else:
        print("Aucun temps trouvé dans le fichier.")

# Exemple d'utilisation
parse_image_times("wfc_sample/out/job.15646.out")

