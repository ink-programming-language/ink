// Translated from solution.cpp.

func read()
{
  var s = 0;
  var t = 1;
  var ch = getchar();
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

var N = (2e5 + 5);

var F = cpp_array(N);

var n: dynamic;

var m: dynamic;

var q: dynamic;

var tot: dynamic;

var U: dynamic;

var D: dynamic;

var a = cpp_array(N, 13);

var ID = cpp_array(13, N);

func cmp(x: dynamic, y: dynamic)
{
  return (a[x][D] > a[y][D]);
}

func main()
{
  m = read();
  tot = cpp_assign(n, "=", read());
  q = read();
  U = (((1 << n)) - 1);
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
    var j = 1;
    while ((j <= m))
    {
      {
        var i = 1;
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
    var i = 1;
    while ((i <= n))
    {
      {
        var S = 0;
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
    var op = read();
    var x = read();
    var y = read();
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
        var i = 1;
        var S = 0;
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
