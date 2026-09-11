// Translated from solution.cpp.

var BIG_NUM: dynamic = cpp_expression("#include <");

var MOD: dynamic = cpp_expression("#include <");

var EPS: dynamic = cpp_expression("#include <s");

var NULL_VAL: dynamic = cpp_expression("#i");

var UNKNOWN: dynamic = cpp_expression("#include <");

var Error: dynamic = cpp_expression("#include");

var NoReturn: dynamic = cpp_expression("#include");

enum Type
{
  enum_field None;
  enum_field Malloc;
  enum_field Free;
  enum_field Clone;
}

class Info
{
  var size: dynamic = cpp_uninitialized();
  var deleted: dynamic = cpp_uninitialized();
}

var value_table: dynamic = cpp_array(26);

var heap_room: dynamic = cpp_uninitialized();

var info_index: dynamic = cpp_uninitialized();

var max_room: dynamic = cpp_uninitialized();

var info: dynamic = cpp_array(10000);

var buf: dynamic = cpp_array(301);

func parse(left: dynamic, right: dynamic) -> dynamic
{
  var index: dynamic = left;
  var depth: dynamic = cpp_uninitialized();
  var close_pos: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  var pre: dynamic = None;
  while ((index <= right))
  {
    var __cpp_switch_1: dynamic = buf[index];
    if (__cpp_switch_1 == cpp_char("("))
    {
      depth = 0;
      {
      var i: dynamic = index;
      while ((i <= right))
      {
      if ((buf[i] == cpp_char("(")))
      {
      depth += 1;
      } else if ((buf[i] == cpp_char(")")))
      {
      depth -= 1;
      if ((depth == 0))
      {
      close_pos = i;
      break;
      }
      }
      i += 1;
      }
      }
      tmp = parse((index + 1), (close_pos - 1));
      if ((tmp == Error))
      {
      return Error;
      }
      var __cpp_switch_2: dynamic = pre;
      if (__cpp_switch_2 == Malloc)
      {
      if ((tmp <= heap_room))
      {
      info[info_index].size = tmp;
      heap_room -= tmp;
      info_index += 1;
      return (info_index - 1);
      } else
      {
      return NULL_VAL;
      }
      break;
      }
      else if (__cpp_switch_2 == Free)
      {
      if ((tmp == NULL_VAL))
      {
      return NoReturn;
      } else if ((((tmp == UNKNOWN) || (tmp >= info_index)) || (info[tmp].deleted == true)))
      {
      return Error;
      } else
      {
      info[tmp].deleted = true;
      heap_room += info[tmp].size;
      return NoReturn;
      }
      break;
      }
      else if (__cpp_switch_2 == Clone)
      {
      if ((tmp == NULL_VAL))
      {
      return NULL_VAL;
      } else if ((((tmp == UNKNOWN) || (tmp >= info_index)) || (info[tmp].deleted == true)))
      {
      return Error;
      } else
      {
      if ((info[tmp].size <= heap_room))
      {
      info[info_index].size = info[tmp].size;
      heap_room -= info[info_index].size;
      info_index += 1;
      return (info_index - 1);
      } else
      {
      return NULL_VAL;
      }
      }
      break;
      }
      else if (__cpp_switch_2 == None)
      {
      return tmp;
      break;
      }
      index = (close_pos + 1);
      pre = None;
      break;
    }
    else if (__cpp_switch_1 == cpp_char("m"))
    {
      pre = Malloc;
      index += 6;
      break;
    }
    else if (__cpp_switch_1 == cpp_char("f"))
    {
      pre = Free;
      index += 4;
      break;
    }
    else if (__cpp_switch_1 == cpp_char("c"))
    {
      pre = Clone;
      index += 5;
      break;
    }
    else
    {
      if (((buf[index] >= cpp_char("A")) && (buf[index] <= cpp_char("Z"))))
      {
      if (((buf[index] == cpp_char("N")) && (buf[(index + 1)] == cpp_char("U"))))
      {
      return NULL_VAL;
      }
      var loc: dynamic = (buf[index] - cpp_char("A"));
      if ((buf[(index + 1)] == cpp_char("=")))
      {
      tmp = parse((index + 2), right);
      value_table[loc] = tmp;
      return value_table[loc];
      } else if ((index == right))
      {
      return value_table[loc];
      }
      } else
      {
      tmp = 0;
      {
      var i: dynamic = index;
      while ((((i <= right) && (buf[i] >= cpp_char("0"))) && (buf[i] <= cpp_char("9"))))
      {
      tmp = ((10 * tmp) + ((buf[i] - cpp_char("0"))));
      if ((tmp > max_room))
      {
      return (max_room + 1);
      }
      i += 1;
      }
      }
      return tmp;
      }
      break;
    }
  }
  return NULL_VAL;
}

func main() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 10000))
    {
      info[i].size = -1;
      info[i].deleted = false;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      value_table[i] = UNKNOWN;
      i += 1;
    }
  }
  scanf("%d", (&heap_room));
  max_room = heap_room;
  var length: dynamic = cpp_uninitialized();
  info_index = 0;
  while ((scanf("%s", buf) != EOF))
  {
    {
      length = 0;
      while ((buf[length] != 0))
      {
        length += 1;
      }
    }
    if ((parse(0, (length - 1)) == Error))
    {
      printf("Error\n");
      return 0;
    }
  }
  var ans: dynamic = 0;
  var FLG: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < info_index))
    {
      if ((info[i].deleted == false))
      {
        FLG = false;
        {
          var k: dynamic = 0;
          while ((k < 26))
          {
            if ((value_table[k] == i))
            {
              FLG = true;
              break;
            }
            k += 1;
          }
        }
        if ((!FLG))
        {
          ans += info[i].size;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
