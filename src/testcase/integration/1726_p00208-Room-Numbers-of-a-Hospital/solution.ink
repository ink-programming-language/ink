// Translated from solution.cpp.

func main()
{
  while (1)
  {
    var a: dynamic;
    var j = 0;
    read(a);
    if ((a == 0))
    {
      break;
    }
    j = a;
    var s = "";
    var b = 0;
    {
      b = 1;
      while (((b * 8) <= a))
      {
        b *= 8;
      }
    }
    {
      var c = b;
      while ((c != 1))
      {
        var o = (a / c);
        var __cpp_switch_1 = (a / c);
        if (__cpp_switch_1 == 0)
        {
        }
        else if (__cpp_switch_1 == 1)
        {
        }
        else if (__cpp_switch_1 == 2)
        {
        }
        else if (__cpp_switch_1 == 3)
        {
          write((a / c));
          break;
        }
        else if (__cpp_switch_1 == 4)
        {
          write(5);
          break;
        }
        else
        {
          write(((a / c) + 2));
          break;
        }
        a -= (c * o);
        c /= 8;
      }
    }
    var __cpp_switch_2 = (j % 8);
    if (__cpp_switch_2 == 0)
    {
    }
    else if (__cpp_switch_2 == 1)
    {
    }
    else if (__cpp_switch_2 == 2)
    {
    }
    else if (__cpp_switch_2 == 3)
    {
      write((j % 8));
      break;
    }
    else if (__cpp_switch_2 == 4)
    {
      write(5);
      break;
    }
    else
    {
      write(((j % 8) + 2));
    }
    write("\n");
  }
}
