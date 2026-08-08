// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var a = cpp_array(10);

func main()
{
  read(n, m);
  memset(a, -1, cpp_sizeof(a));
  var f = 1;
  while (cpp_update(m, "--"))
  {
    var pos: dynamic;
    var c: dynamic;
    read(pos, c);
    if (((a[pos] != -1) && (c != a[pos])))
    {
      f = 0;
      break;
    } else
    {
      a[pos] = c;
    }
  }
  if ((n == 1))
  {
    if ((a[1] == -1))
    {
      a[1] = 0;
    }
  } else
  {
    if ((a[1] == 0))
    {
      f = 0;
    }
    if ((a[1] == -1))
    {
      a[1] = 1;
    }
    {
      var i = 2;
      while ((i <= n))
      {
        if ((a[i] == -1))
        {
          a[i] = 0;
        }
        i += 1;
      }
    }
  }
  if ((!f))
  {
    write("-1", "\n");
  } else
  {
    {
      var i = 1;
      while ((i <= n))
      {
        write(a[i]);
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
