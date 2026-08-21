import re

with open('src/app/IupacNamer.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# patch groupRank functions
orig_rank = '''                case GroupType::ARSONIC_ACID: return 5;
                case GroupType::BORONIC_ACID: return 6;
                case GroupType::ESTER: return 7;
                case GroupType::ACYL_HALIDE: return 8;
                case GroupType::SULFONYL_HALIDE: return 9;
                case GroupType::AMIDE: return 10;
                case GroupType::HYDRAZIDE: return 11;
                case GroupType::NITRILE: return 12;
                case GroupType::ALDEHYDE: return 13;
                case GroupType::THIAL: return 14;
                case GroupType::KETONE: return 15;
                case GroupType::THIONE: return 16;
                case GroupType::ALCOHOL: return 17;
                case GroupType::THIOL: return 18;
                case GroupType::SELENOL: return 19;
                case GroupType::TELLUROL: return 20;
                case GroupType::HYDROPEROXIDE: return 21;
                case GroupType::AMINE: return 22;
                case GroupType::IMINE: return 23;
                case GroupType::PHOSPHINE: return 24;
                default: return 25;'''

new_rank = '''                case GroupType::PHOSPHONIC_DIHALIDE: return 5;
                case GroupType::ARSONIC_ACID: return 6;
                case GroupType::BORONIC_ACID: return 7;
                case GroupType::ESTER: return 8;
                case GroupType::ACYL_HALIDE: return 9;
                case GroupType::SULFONYL_HALIDE: return 10;
                case GroupType::AMIDE: return 11;
                case GroupType::HYDRAZIDE: return 12;
                case GroupType::NITRILE: return 13;
                case GroupType::ALDEHYDE: return 14;
                case GroupType::THIAL: return 15;
                case GroupType::KETONE: return 16;
                case GroupType::THIONE: return 17;
                case GroupType::ALCOHOL: return 18;
                case GroupType::THIOL: return 19;
                case GroupType::SELENOL: return 20;
                case GroupType::TELLUROL: return 21;
                case GroupType::HYDROPEROXIDE: return 22;
                case GroupType::AMINE: return 23;
                case GroupType::IMINE: return 24;
                case GroupType::PHOSPHINE: return 25;
                default: return 26;'''

content = content.replace(orig_rank, new_rank)

orig_rank2 = '''            case GroupType::ARSONIC_ACID: return 5;
            case GroupType::BORONIC_ACID: return 6;
            case GroupType::ESTER: return 7;
            case GroupType::ACYL_HALIDE: return 8;
            case GroupType::SULFONYL_HALIDE: return 9;
            case GroupType::AMIDE: return 10;
            case GroupType::HYDRAZIDE: return 11;
            case GroupType::NITRILE: return 12;
            case GroupType::ALDEHYDE: return 13;
            case GroupType::THIAL: return 14;
            case GroupType::KETONE: return 15;
            case GroupType::THIONE: return 16;
            case GroupType::ALCOHOL: return 17;
            case GroupType::THIOL: return 18;
            case GroupType::SELENOL: return 19;
            case GroupType::TELLUROL: return 20;
            case GroupType::HYDROPEROXIDE: return 21;
            case GroupType::AMINE: return 22;
            case GroupType::IMINE: return 23;
            case GroupType::PHOSPHINE: return 24;
            default: return 25;'''

new_rank2 = '''            case GroupType::PHOSPHONIC_DIHALIDE: return 5;
            case GroupType::ARSONIC_ACID: return 6;
            case GroupType::BORONIC_ACID: return 7;
            case GroupType::ESTER: return 8;
            case GroupType::ACYL_HALIDE: return 9;
            case GroupType::SULFONYL_HALIDE: return 10;
            case GroupType::AMIDE: return 11;
            case GroupType::HYDRAZIDE: return 12;
            case GroupType::NITRILE: return 13;
            case GroupType::ALDEHYDE: return 14;
            case GroupType::THIAL: return 15;
            case GroupType::KETONE: return 16;
            case GroupType::THIONE: return 17;
            case GroupType::ALCOHOL: return 18;
            case GroupType::THIOL: return 19;
            case GroupType::SELENOL: return 20;
            case GroupType::TELLUROL: return 21;
            case GroupType::HYDROPEROXIDE: return 22;
            case GroupType::AMINE: return 23;
            case GroupType::IMINE: return 24;
            case GroupType::PHOSPHINE: return 25;
            default: return 26;'''

content = content.replace(orig_rank2, new_rank2)

with open('src/app/IupacNamer.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Patching done!")
