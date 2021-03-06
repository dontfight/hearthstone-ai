﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace HearthstoneAI.LogWatcher.SubParsers
{
    class ParserUtilities
    {
        public static readonly Regex EntityRegex =
            new Regex(@"(?=id=(?<id>(\d+)))(?=name=(?<name>(\w+)))?(?=zone=(?<zone>(\w+)))?(?=zonePos=(?<zonePos>(\d+)))?(?=cardId=(?<cardId>(\w+)))?(?=player=(?<player>(\d+)))?(?=type=(?<type>(\w+)))?");

        static public int GetEntityIdFromRawString(State.Game game_state, string entity_raw)
        {
            string entity_str;
            return GetEntityIdFromRawString(game_state, entity_raw, out entity_str);
        }

        static public int GetEntityIdFromRawString(State.Game game_state, string entity_raw, out string entity_str)
        {
            entity_raw = entity_raw.Replace("UNKNOWN ENTITY ", "");
            int entityId = -1;
            entity_str = "";

            // Get entity id - Method 1
            if (entity_raw.StartsWith("[") && EntityRegex.IsMatch(entity_raw))
            {
                entityId = int.Parse(EntityRegex.Match(entity_raw).Groups["id"].Value);
            }

            // Get entity id - Method 2
            if (entityId < 0)
            {
                if (!int.TryParse(entity_raw, out entityId)) entityId = -1;
            }

            // Get entity id - Method 3
            if (entityId < 0)
            {
                string entity_str_try = entity_raw; 
                var entity = game_state.Entities.Items.FirstOrDefault(x => x.Value.Name == entity_str_try);
                if (entity.Value != null)
                {
                    entityId = entity.Key;
                    entity_str = entity_str_try;
                }
                else
                {
                    entity_str_try = "UNKNOWN HUMAN PLAYER";
                    entity = game_state.Entities.Items.FirstOrDefault(x => x.Value.Name == entity_str_try);
                    if (entity.Value != null)
                    {
                        entityId = entity.Key;
                        entity_str = entity_str_try;
                        game_state.ChangeEntityName(entityId, entity_str_try);
                    }
                    else
                    {
                        entity_str = entity_raw;
                    }
                }
            }

            return entityId;
        }

        static public IEnumerable<bool> TrySubParsers(List<Func<IEnumerable<bool>>> parsers)
        {
            foreach (var parser in parsers)
            {
                bool matched = false;
                foreach (var ret in parser())
                {
                    matched = true;
                    yield return ret;
                }
                
                if (matched) yield break; // match the first parser
            }
        }
    }
}
