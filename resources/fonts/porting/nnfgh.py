import os
import re
from PIL import Image
from pathlib import Path

def parse_fnt_file(fnt_path):
    """Parse .fnt file and extract all parameters"""
    with open(fnt_path, 'r', encoding='utf-8') as f:
        content = f.read()
    return content

def scale_fnt_content(content, scale_factor=2.0):
    """Scale all numeric values in .fnt file by scale_factor"""
    
    # Parameters that should be scaled (integers)
    int_params = [
        'size', 'lineHeight', 'base', 'scaleW', 'scaleH',
        'x', 'y', 'width', 'height', 'xoffset', 'yoffset', 'xadvance',
        'first', 'second', 'amount'
    ]
    
    lines = content.split('\n')
    scaled_lines = []
    
    for line in lines:
        if not line.strip():
            scaled_lines.append(line)
            continue
            
        # Handle each line type
        if line.startswith('info'):
            # Scale size parameter
            line = re.sub(r'size=(\d+)', lambda m: f'size={int(int(m.group(1)) * scale_factor)}', line)
        
        elif line.startswith('common'):
            # Scale lineHeight, base, scaleW, scaleH
            for param in ['lineHeight', 'base', 'scaleW', 'scaleH']:
                line = re.sub(
                    rf'{param}=(\d+)',
                    lambda m: f'{param}={int(int(m.group(1)) * scale_factor)}',
                    line
                )
        
        elif line.startswith('char '):
            # Scale all character metrics
            for param in ['x', 'y', 'width', 'height', 'xoffset', 'yoffset', 'xadvance']:
                line = re.sub(
                    rf'{param}=(-?\d+)',
                    lambda m: f'{param}={int(int(m.group(1)) * scale_factor)}',
                    line
                )
        
        elif line.startswith('kerning '):
            # Scale kerning amount
            line = re.sub(
                r'amount=(-?\d+)',
                lambda m: f'amount={int(int(m.group(1)) * scale_factor)}',
                line
            )
        
        scaled_lines.append(line)
    
    return '\n'.join(scaled_lines)

def update_png_reference(content, old_suffix, new_suffix):
    """Update the PNG file reference in .fnt content"""
    return re.sub(
        rf'file="([^"]+){old_suffix}\.png"',
        rf'file="\1{new_suffix}.png"',
        content
    )

def scale_png_image(png_path, output_path, scale_factor=2.0):
    """Scale PNG image to half size using high-quality resampling"""
    img = Image.open(png_path)
    new_width = int(img.width * scale_factor)
    new_height = int(img.height * scale_factor)
    
    # Use LANCZOS for high-quality downsampling
    scaled_img = img.resize((new_width, new_height), Image.Resampling.NEAREST)
    scaled_img.save(output_path)
    print(f"  Scaled PNG: {png_path.name} -> {output_path.name}")

def port_font(base_name, input_suffix, output_suffix, fonts_dir):
    """Port a font from one quality to another"""
    input_fnt = fonts_dir / f"{base_name}{input_suffix}.fnt"
    input_png = fonts_dir / f"{base_name}{input_suffix}.png"
    output_fnt = fonts_dir / f"{base_name}{output_suffix}.fnt"
    output_png = fonts_dir / f"{base_name}{output_suffix}.png"
    
    # Check if input files exist
    if not input_fnt.exists() or not input_png.exists():
        return False
    
    # Check if output files already exist
    if output_fnt.exists() or output_png.exists():
        print(f"⚠ Skipping {base_name}{input_suffix} -> {base_name}{output_suffix} (output already exists)")
        return False
    
    print(f"\n📝 Converting: {base_name}{input_suffix} -> {base_name}{output_suffix}")
    
    # Parse and scale .fnt file
    fnt_content = parse_fnt_file(input_fnt)
    scaled_content = scale_fnt_content(fnt_content, scale_factor=2.0)
    
    # Update PNG reference
    png_ref_old = input_suffix if input_suffix else ""
    png_ref_new = output_suffix if output_suffix else ""
    scaled_content = update_png_reference(scaled_content, png_ref_old, png_ref_new)
    
    # Save scaled .fnt file
    with open(output_fnt, 'w', encoding='utf-8') as f:
        f.write(scaled_content)
    print(f"  Saved FNT: {output_fnt.name}")
    
    # Scale and save PNG image
    scale_png_image(input_png, output_png, scale_factor=2.0)
    
    return True

def main():
    """Main function to process all fonts in the fonts directory"""
    fonts_dir = Path("fonts")
    
    if not fonts_dir.exists():
        print("❌ Error: 'fonts' directory not found!")
        print("Please create a 'fonts' folder and place your .fnt and .png files inside.")
        return
    
    print("🎮 Geometry Dash Font Quality Porter")
    print("=" * 50)
    
    # Get all .fnt files
    fnt_files = list(fonts_dir.glob("*.fnt"))
    
    if not fnt_files:
        print("❌ No .fnt files found in the fonts directory!")
        return
    
    # Extract base names and their suffixes
    fonts_to_process = {}
    
    for fnt_file in fnt_files:
        name = fnt_file.stem
        
        if name.endswith('-uhd'):
            base_name = name[:-4]
            if base_name not in fonts_to_process:
                fonts_to_process[base_name] = set()
            fonts_to_process[base_name].add('uhd')
        elif name.endswith('-hd'):
            base_name = name[:-3]
            if base_name not in fonts_to_process:
                fonts_to_process[base_name] = set()
            fonts_to_process[base_name].add('hd')
        else:
            # Low quality (no suffix)
            if name not in fonts_to_process:
                fonts_to_process[name] = set()
            fonts_to_process[name].add('low')
    
    conversions_made = 0
    
    # Process each font
    for base_name, qualities in sorted(fonts_to_process.items()):
        print(f"\n🔍 Found font: {base_name}")
        print(f"   Available qualities: {', '.join(sorted(qualities))}")
        
        # UHD -> HD conversion
        if 'uhd' in qualities:
            if port_font(base_name, '-uhd', '-hd', fonts_dir):
                conversions_made += 1
        
        # HD -> Low conversion
        if 'hd' in qualities:
            if port_font(base_name, '-hd', '', fonts_dir):
                conversions_made += 1
    
    print("\n" + "=" * 50)
    print(f"✅ Done! Made {conversions_made} conversion(s).")
    
    if conversions_made == 0:
        print("\nℹ️  No conversions were needed. All lower quality versions already exist.")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"\n❌ Error: {e}")
        import traceback
        traceback.print_exc()