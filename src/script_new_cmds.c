#include "../include/types.h"
#include "../include/script.h"
#include "../include/repel.h"
#include "../include/constants/file.h"

// Small helpers to avoid taking the address of literals everywhere (saves ROM in big switches)
static inline void SetMonU8(struct PartyPokemon *pp, int field, u8 value)
{
    SetMonData(pp, field, &value);
}

static inline void SetMonU16(struct PartyPokemon *pp, int field, u16 value)
{
    SetMonData(pp, field, &value);
}


#define SCRIPT_NEW_CMD_REPEL_USE    0

#define SCRIPT_NEW_CMD_IV_TUTOR 1

#define SCRIPT_NEW_CMD_MAX          256

BOOL Script_RunNewCmd(SCRIPTCONTEXT *ctx) {
    u8 sw = ScriptReadByte(ctx);
    u16 UNUSED arg0 = ScriptReadHalfword(ctx);

    switch (sw) {
        case SCRIPT_NEW_CMD_REPEL_USE:;
#ifdef IMPLEMENT_REUSABLE_REPELS
            u16 most_recent_repel = Repel_GetMostRecent();
            SetScriptVar(arg0, most_recent_repel);
            Repel_Use(most_recent_repel, HEAPID_MAIN_HEAP);
#endif
            break;

case SCRIPT_NEW_CMD_IV_TUTOR:;
#ifdef IMPLEMENT_IV_TUTOR
            FieldSystem *fsys = ctx->fsys;
            struct PartyPokemon *pp;
            struct Party *party = SaveData_GetPlayerPartyPtr(fsys->savedata);
                        // (locals removed; use constants/tables below)
u8 pos = GetScriptVar(0x8008);
            pp = Party_GetMonByIndex (party, pos);
                        // Table-driven mapping (smaller than a huge switch/jump-table)
            static const u8 sIvFields[] = {
                MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV,
                MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV, MON_DATA_SPEED_IV
            };
            static const u8 sEvFields[] = {
                MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV,
                MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV, MON_DATA_SPEED_EV
            };

            static const u16 sBallByArg0[] = {
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                11, 12, 13, 14, 15, 16, 492, 493, 494, 495,
                496, 497, 498, 499, 500
            }; // arg0 13..37

            static const u16 sAbilityByArg0[] = {
                1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                11, 12, 13, 14, 15, 50, 17, 18, 19, 20,
                21, 22, 24, 26, 27, 28, 29, 30, 31, 32,
                33, 34, 36, 37, 38, 39, 40, 41, 42, 43,
                44, 45, 46, 47, 48, 49, 51, 52, 53, 55,
                56, 60, 61, 62, 63, 64, 65, 66, 67, 68,
                69, 70, 71, 72, 73, 74, 75, 77, 78, 79,
                80, 81, 82, 83, 84, 85, 87, 88, 89, 90,
                91, 92, 93, 94, 95, 97, 98, 99, 101, 102,
                104, 105, 106, 107, 108, 109, 110, 111, 113, 114,
                115, 116, 117, 119, 120, 124, 125, 127, 128, 130,
                131, 133, 136, 139, 140, 142, 143, 144, 145, 146,
                147, 148, 151, 153, 154, 155, 156, 157, 159, 172,
                173, 174, 181, 182, 201, 202, 291, 292, 296
            }; // arg0 38..176

            if (arg0 == 0) {
                for (u32 i = 0; i < (u32)(sizeof(sIvFields) / sizeof(sIvFields[0])); i++)
                    SetMonU8(pp, sIvFields[i], 31);
            } else if (arg0 == 1) {
                SetMonU8(pp, MON_DATA_ATK_IV, 0);
            } else if (arg0 == 2) {
                SetMonU8(pp, MON_DATA_SPATK_IV, 0);
            } else if (arg0 == 3) {
                SetMonU8(pp, MON_DATA_SPEED_IV, 0);
            } else if (arg0 >= 4 && arg0 <= 9) {
                // EVs: arg0 4..9 => HP/ATK/DEF/SPEED/SPATK/SPDEF (same order as sEvFields)
                SetMonU8(pp, sEvFields[arg0 - 4], 252);
            } else if (arg0 == 10) {
                for (u32 i = 0; i < (u32)(sizeof(sEvFields) / sizeof(sEvFields[0])); i++)
                    SetMonU8(pp, sEvFields[i], 252);
            } else if (arg0 == 11) {
                SetMonU8(pp, MON_DATA_FRIENDSHIP, 255);
            } else if (arg0 == 12) {
                SetMonU8(pp, MON_DATA_HP_EV, 0);
            } else if (arg0 >= 13 && arg0 <= 37) {
                SetMonU16(pp, MON_DATA_POKEBALL, sBallByArg0[arg0 - 13]);
            } else if (arg0 >= 38 && arg0 <= 176) {
                SetMonU16(pp, MON_DATA_ABILITY, sAbilityByArg0[arg0 - 38]);
            } else if (arg0 >= 177 && arg0 <= 181) {
                // EVs: arg0 177..181 => ATK/DEF/SPEED/SPATK/SPDEF (same order as sEvFields)
                SetMonU8(pp, sEvFields[arg0 - 176], 0);
            }

            RecalcPartyPokemonStats(pp);
#endif
            break;

        default: break;
    }

    return FALSE;
}