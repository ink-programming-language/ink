// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  while (true)
  {
    read(n);
    if ((n == 0))
    {
      break;
    }
    var jew: dynamic = cpp_array(21, 21);
    {
      var i: dynamic = 0;
      while ((i < 441))
      {
        jew[(i % 21)][(i / 21)] = true;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(x, y);
        jew[x][y] = false;
        i += 1;
      }
    }
    read(m);
    var dir: dynamic = cpp_array(m);
    var len: dynamic = cpp_array(m);
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        read(dir[i], len[i]);
        i += 1;
      }
    }
    x = 10;
    y = 10;
    var correct: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        while ((len[i] > 0))
        {
          var __cpp_switch_1: dynamic = dir[i];
          if (__cpp_switch_1 == cpp_char("N"))
          {
            y += 1;
            break;
          }
          else if (__cpp_switch_1 == cpp_char("E"))
          {
            x += 1;
            break;
          }
          else if (__cpp_switch_1 == cpp_char("S"))
          {
            y -= 1;
            break;
          }
          else if (__cpp_switch_1 == cpp_char("W"))
          {
            x -= 1;
            break;
          }
          if ((!jew[x][y]))
          {
            jew[x][y] = true;
            n -= 1;
          }
          len[i] -= 1;
        }
        i += 1;
      }
    }
    if (n)
    {
      write("No", "\n");
    } else
    {
      write("Yes", "\n");
    }
  }
  return 0;
}
