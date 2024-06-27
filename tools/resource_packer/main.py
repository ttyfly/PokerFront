import argparse

from text_packer import TextPacker


def start_packing(project_dir):
    packers = [
        TextPacker(project_dir),
    ]


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--project-dir', type=str, default='../..')
    parser.add_argument('--output-bin-dir', type=str, default='../..')
    parser.add_argument('--output-code-dir', type=str, default='../..')
    args = parser.parse_args()
    start_packing(args.project_dir)
