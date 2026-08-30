#include "../include/config.h"
#include "../include/constants/file.h"
#include "../include/pokemon.h"
#include "../include/repel.h"
#include "../include/roamer.h"
#include "../include/script.h"
#include "../include/types.h"

// Small helpers to avoid taking the address of literals everywhere (saves ROM in big switches)
static inline void SetMonU8(struct PartyPokemon *pp, int field, u8 value)
{
    SetMonData(pp, field, &value);
}

static inline void SetMonU16(struct PartyPokemon *pp, int field, u16 value)
{
    SetMonData(pp, field, &value);
}

static inline void SetMonU32(struct PartyPokemon *pp, int field, u32 value)
{
    SetMonData(pp, field, &value);
}

// Queued PID choices. Nature, gender and shininess are all read from the same
// 32-bit personality value, so they cannot be set one at a time - each write
// disturbs the others. These hold the player's picks until arg0 243 searches
// for a single PID that satisfies all of them at once. 0xFF means "not set".
static u8 sPendingNature = 0xFF;
static u8 sPendingGender = 0xFF;
static u8 sPendingShiny  = 0xFF;

#define PID_SUBSTRUCT_MASK 0x0003E000u // selects data block order - must never move

#define SCRIPT_NEW_CMD_REPEL_USE        0
#define SCRIPT_NEW_CMD_IV_TUTOR         1
#define SCRIPT_NEW_CMD_IV_TUTOR_INDEXED 2
#define SCRIPT_NEW_CMD_MAX              256

BOOL Script_RunNewCmd(SCRIPTCONTEXT *ctx)
{
    u8 sw = ScriptReadByte(ctx);
    u16 UNUSED arg0 = ScriptReadHalfword(ctx);

#ifdef IMPLEMENT_IV_TUTOR
    // Indexed form: arg0 is the FIRST value of the menu, and the option the
    // player picked is read from 0x800C. Lets one menu cover every one of its
    // entries, instead of needing a separate script function per option.
    // The script must catch a cancelled menu (0x800C == 0) before calling this.
    if (sw == SCRIPT_NEW_CMD_IV_TUTOR_INDEXED) {
        arg0 = arg0 + (u16)GetScriptVar(0x800C) - 1;
        sw = SCRIPT_NEW_CMD_IV_TUTOR;
    }
#endif

    switch (sw) {
    case SCRIPT_NEW_CMD_REPEL_USE: {
#ifdef IMPLEMENT_REUSABLE_REPELS
        u16 most_recent_repel = Repel_GetMostRecent();
        SetScriptVar(arg0, most_recent_repel);
        Repel_Use(most_recent_repel, HEAPID_MAIN_HEAP);
#endif
        break;
    }

    case SCRIPT_NEW_CMD_IV_TUTOR: {
#ifdef IMPLEMENT_IV_TUTOR
        // Queue commands: record the choice, no Pokemon needed yet.
        if (arg0 >= 214 && arg0 <= 238) {
            sPendingNature = arg0 - 214;
            break;
        }
        if (arg0 == 239 || arg0 == 240) {
            sPendingGender = (arg0 == 239) ? POKEMON_GENDER_MALE : POKEMON_GENDER_FEMALE;
            break;
        }
        if (arg0 == 241 || arg0 == 242) {
            sPendingShiny = (arg0 == 241);
            break;
        }
        if (arg0 == 244) {
            sPendingNature = 0xFF;
            sPendingGender = 0xFF;
            sPendingShiny  = 0xFF;
            break;
        }

        FieldSystem *fsys = ctx->fsys;
        struct PartyPokemon *pp;
        struct Party *party = SaveData_GetPlayerPartyPtr(fsys->savedata);
        u8 pos = GetScriptVar(0x8008);

        // The script already guards this (255 = cancelled, species 0 = empty slot),
        // but a bad var value here would write through a junk pointer.
        if (pos >= 6) {
            break;
        }

        pp = Party_GetMonByIndex(party, pos);
        if (pp == NULL) {
            break;
        }

        static const u8 sIvFields[] = {
            MON_DATA_HP_IV, MON_DATA_ATK_IV, MON_DATA_DEF_IV,
            MON_DATA_SPATK_IV, MON_DATA_SPDEF_IV, MON_DATA_SPEED_IV
        };
        static const u8 sEvFields[] = {
            MON_DATA_HP_EV, MON_DATA_ATK_EV, MON_DATA_DEF_EV,
            MON_DATA_SPATK_EV, MON_DATA_SPDEF_EV, MON_DATA_SPEED_EV
        };

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

        static const u16 sBallByArg0[] = {
            1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
            11, 12, 13, 14, 15, 16, 492, 493, 494, 495,
            496, 497, 498, 499, 500
        }; // arg0 13..37

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
            SetMonU8(pp, sEvFields[arg0 - 4], 252);
        } else if (arg0 == 10) {
            for (u32 i = 0; i < (u32)(sizeof(sEvFields) / sizeof(sEvFields[0])); i++)
                SetMonU8(pp, sEvFields[i], 252);
        } else if (arg0 == 11) {
            SetMonU8(pp, MON_DATA_FRIENDSHIP, 255);
        } else if (arg0 == 245) {
            SetMonU8(pp, MON_DATA_FRIENDSHIP, 0);
        } else if (arg0 == 12) {
            SetMonU8(pp, MON_DATA_HP_EV, 0);
        } else if (arg0 >= 13 && arg0 <= 37) {
            SetMonU16(pp, MON_DATA_POKEBALL, sBallByArg0[arg0 - 13]);
        } else if (arg0 >= 38 && arg0 <= 176) {
            // Any ability, including ones the species cannot normally have.
            SetMonU16(pp, MON_DATA_ABILITY, sAbilityByArg0[arg0 - 38]);
        } else if (arg0 >= 177 && arg0 <= 181) {
            SetMonU8(pp, sEvFields[arg0 - 176], 0);

        } else if (arg0 >= 182 && arg0 <= 206) {
            // Nature. Only bits 0-12 are touched, so the substructure-order
            // bits (0x0003E000) can never move - changing those makes the game
            // read the data blocks in the wrong order, which shows up as a
            // completely different species and level.
            u32 pid  = GetMonData(pp, MON_DATA_PERSONALITY, NULL);
            u32 base = pid & ~0x1FFFu;
            u32 lo   = pid & 0x1FFFu;
            u32 want = arg0 - 182;

            lo += (want + 25 - ((base + lo) % 25)) % 25;
            if (lo > 0x1FFF) {
                lo -= 25; // back inside 13 bits; -25 leaves the nature alone
            }

            SetMonU32(pp, MON_DATA_PERSONALITY, base + lo);

        } else if (arg0 == 207 || arg0 == 208) {
            // Shininess (207 = make shiny, 208 = remove shiny).
            u32 pid  = GetMonData(pp, MON_DATA_PERSONALITY, NULL);
            u32 otid = GetMonData(pp, MON_DATA_OTID, NULL);

            if (arg0 == 207) {
                // Engine helper - produces a shiny PID without moving 0x3E000.
                pid = GenerateShinyPIDKeepSubstructuresIntact(otid, pid);
            } else if (SHINY_CHECK(otid, pid)) {
                // Flipping bit 31 shifts the shiny value by 0x8000, far above
                // SHINY_ODDS, and sits outside the substructure mask.
                pid ^= 0x80000000u;
            }

            SetMonU32(pp, MON_DATA_PERSONALITY, pid);

        } else if (arg0 == 209 || arg0 == 210) {
            // Gender. Walks the low 13 bits in steps of 25, so both the nature
            // and the substructure bits stay put. gcd(25, 256) == 1, so every
            // possible low byte is reachable.
            u32 pid  = GetMonData(pp, MON_DATA_PERSONALITY, NULL);
            u32 base = pid & ~0x1FFFu;
            u32 lo   = pid & 0x1FFFu;
            u8 want  = (arg0 == 209) ? POKEMON_GENDER_MALE : POKEMON_GENDER_FEMALE;
            BOOL found = FALSE;

            for (u32 cand = lo % 25; cand <= 0x1FFF; cand += 25) {
                SetMonU32(pp, MON_DATA_PERSONALITY, base + cand);
                if (GetMonData(pp, MON_DATA_GENDER, NULL) == want) {
                    found = TRUE;
                    break;
                }
            }

            if (!found) {
                // Species cannot be this gender - put the original PID back.
                SetMonU32(pp, MON_DATA_PERSONALITY, pid);
            }
        } else if (arg0 >= 211 && arg0 <= 213) {
            // Ability slot (211 = ability 1, 212 = ability 2, 213 = hidden).
            // Writing MON_DATA_ABILITY directly gets overwritten whenever the
            // game recalculates the ability from species + slot, e.g. on
            // evolution. SetBoxMonAbility reads these two bits instead and
            // writes MON_DATA_ABILITY itself, so the choice sticks.
            //
            // From SetBoxMonAbility: ability 1 is used when the swap bit
            // matches (pid & 1), ability 2 when it differs.
            u32 pid = GetMonData(pp, MON_DATA_PERSONALITY, NULL);
            u8  hid = GetMonData(pp, MON_DATA_RESERVED_113, NULL);
            u16 slt = GetMonData(pp, MON_DATA_RESERVED_114, NULL);

            if (arg0 == 213) {
                hid |= DUMMY_P2_1_HIDDEN_ABILITY_MASK;
            } else {
                BOOL setSwap = (arg0 == 211) ? ((pid & 1) != 0) : ((pid & 1) == 0);

                hid &= ~DUMMY_P2_1_HIDDEN_ABILITY_MASK;
                if (setSwap) {
                    slt |= DUMMY_P2_2_CHANGE_ABILITY_SLOT;
                } else {
                    slt &= ~DUMMY_P2_2_CHANGE_ABILITY_SLOT;
                }
            }

            SetMonData(pp, MON_DATA_RESERVED_113, (u8 *)&hid);
            SetMonData(pp, MON_DATA_RESERVED_114, (u8 *)&slt);
            SetBoxMonAbility(&pp->box);

        } else if (arg0 == 243) {
            // Apply every queued choice at once. Hunts for one PID that
            // satisfies all of them while leaving the substructure bits alone.
            // Nature and shininess are pure arithmetic, so they are checked
            // first - only the handful of survivors pay for the gender check,
            // which has to write the PID and read the result back.
            u32 orig = GetMonData(pp, MON_DATA_PERSONALITY, NULL);
            u32 otid = GetMonData(pp, MON_DATA_OTID, NULL);
            u32 keep = orig & PID_SUBSTRUCT_MASK;
            u32 seed = orig ^ 0x9E3779B9u;
            BOOL found = FALSE;

            for (u32 i = 0; i < 4000000u; i++) {
                u32 cand;

                seed = (seed * 1103515245u) + 12345u;
                cand = (seed & ~PID_SUBSTRUCT_MASK) | keep;

                if (sPendingNature != 0xFF && (cand % 25) != sPendingNature) {
                    continue;
                }
                if (sPendingShiny != 0xFF
                 && ((SHINY_CHECK(otid, cand) != 0) != (sPendingShiny != 0))) {
                    continue;
                }
                if (sPendingGender != 0xFF) {
                    SetMonU32(pp, MON_DATA_PERSONALITY, cand);
                    if (GetMonData(pp, MON_DATA_GENDER, NULL) != sPendingGender) {
                        continue;
                    }
                }

                SetMonU32(pp, MON_DATA_PERSONALITY, cand);
                found = TRUE;
                break;
            }

            if (!found) {
                // Nothing matched - most likely an impossible gender for this
                // species. Leave the Pokemon exactly as it was.
                SetMonU32(pp, MON_DATA_PERSONALITY, orig);
            }

            sPendingNature = 0xFF;
            sPendingGender = 0xFF;
            sPendingShiny  = 0xFF;
        }

        RecalcPartyPokemonStats(pp);
#endif
        break;
    }

    default:
        break;
    }

    return FALSE;
}

#ifdef EXPAND_ROAMERS
BOOL LONG_CALL ScrCmd_CreateRoamer(SCRIPTCONTEXT *ctx)
{
    u8 roamerNo = ScriptReadByte(ctx);
    Save_CreateRoamerByID(ctx->fsys->savedata, roamerNo);
    return FALSE;
}
#endif // EXPAND_ROAMERS
