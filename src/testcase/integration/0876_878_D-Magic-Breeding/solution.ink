// Translated from solution.cpp.

func read() -> dynamic
{
  var s: dynamic = 0;
  var t: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      t = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    s = ((((s << 3)) + ((s << 1))) + ((ch ^ 48)));
    ch = getchar();
  }
  return (s * t);
}

var N: dynamic = (2e5 + 5);

var F: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var U: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(N, 13);

var ID: dynamic = cpp_array(13, N);

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return (a[x][D] > a[y][D]);
}

func main() -> dynamic
{
  m = read();
  tot = cpp_assign(n, "=", read());
  q = read();
  U = (((1 << n)) - 1);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          a[i][j] = read();
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((j <= m))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          ID[j][i] = i;
          i += 1;
        }
      }
      D = j;
      sort((ID[j] + 1), ((ID[j] + n) + 1), cmp);
      j += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var S: dynamic = 0;
        while ((S <= U))
        {
          if (((S >> ((i - 1))) & 1))
          {
            F[i][S] = 1;
          }
          S += 1;
        }
      }
      i += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var op: dynamic = read();
    var x: dynamic = read();
    var y: dynamic = read();
    if ((op == 1))
    {
      F[cpp_update(tot, "++")] = (F[x] | F[y]);
    }
    if ((op == 2))
    {
      F[cpp_update(tot, "++")] = (F[x] & F[y]);
    }
    if ((op == 3))
    {
      {
        var i: dynamic = 1;
        var S: dynamic = 0;
        while ((i <= n))
        {
          S |= ((1 << (ID[y][i] - 1)));
          if (F[x][S])
          {
            printf("%d\n", a[ID[y][i]][y]);
            break;
          }
          i += 1;
        }
      }
    }
  }
  return 0;
}
