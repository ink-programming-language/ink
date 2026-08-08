// Translated from solution.cpp.

func R(x: dynamic)
{
  read(x);
}

func R(x: dynamic)
{
  scanf("%d", (&x));
}

func R(x: dynamic)
{
  scanf("%lld", (&x));
}

func R(x: dynamic)
{
  scanf("%lf", (&x));
}

func R(x: dynamic)
{
  scanf(" %c", (&x));
}

func R(x: dynamic)
{
  scanf("%s", x);
}

func R()
{
}

func R(head: dynamic, tail: dynamic...)
{
  R(head);
  R(cpp_expand(tail));
}

func W(x: dynamic)
{
  write(x);
}

func W(x: dynamic)
{
  printf("%d", x);
}

func W(x: dynamic)
{
  printf("%lld", x);
}

func W(x: dynamic)
{
  printf("%.16f", x);
}

func W(x: dynamic)
{
  putchar(x);
}

func W(x: dynamic)
{
  printf("%s", x);
}

func W(x: dynamic)
{
  W(x.first);
  putchar(cpp_char(" "));
  W(x.second);
}

func W(x: dynamic)
{
  {
    var i = x.begin();
    while ((i != x.end()))
    {
      if ((i != x.cbegin()))
      {
        putchar(cpp_char(" "));
      }
      W((*cpp_update(i, "++")));
    }
  }
}

func W()
{
}

func W(head: dynamic, tail: dynamic...)
{
  W(head);
  putchar(if (cpp_sizeof(tail)) cpp_char(" ") else cpp_char("\n"));
  W(cpp_expand(tail));
}

var MOD = (1e9 + 7);

func ADD(x: dynamic, v: dynamic)
{
  x = (((x + v)) % MOD);
  if ((x < 0))
  {
    x += MOD;
  }
}

var SIZE = (1e6 + 10);

class Union_Find
{
  var d: dynamic = cpp_array(SIZE);
  var num: dynamic = cpp_array(SIZE);
  func init(n: dynamic)
  {
      {
        var i = 0;
        while ((i < (n)))
        {
          d[i] = i;
          num[i] = 1;
          i += 1;
        }
      }
    }
  func find(x: dynamic)
  {
      var y = x;
      var z = x;
      while ((y != d[y]))
      {
        y = d[y];
      }
      while ((x != y))
      {
        x = d[x];
        d[z] = y;
        z = x;
      }
      return y;
    }
  func is_root(x: dynamic)
  {
      return (d[x] == x);
    }
  func uu(x: dynamic, y: dynamic)
  {
      x = find(x);
      y = find(y);
      if ((x == y))
      {
        return 0;
      }
      if ((num[x] > num[y]))
      {
        swap(x, y);
      }
      num[y] += num[x];
      d[x] = y;
      return 1;
    }
}

var U: dynamic;

func C2(x: dynamic)
{
  return ((x * ((x - 1))) / 2);
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  R(n, m);
  U.init(n);
  {
    var i = 0;
    while ((i < (m)))
    {
      var x: dynamic;
      var y: dynamic;
      R(x, y);
      x -= 1;
      y -= 1;
      U.uu(x, y);
      i += 1;
    }
  }
  var res = 0;
  {
    var i = 0;
    while ((i < (n)))
    {
      if (U.is_root(i))
      {
        res += C2(U.num[i]);
      }
      i += 1;
    }
  }
  W(if ((res == m)) "YES" else "NO");
  return 0;
}
