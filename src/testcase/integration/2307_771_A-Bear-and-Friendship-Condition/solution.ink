// Translated from solution.cpp.

func R(x: dynamic) -> dynamic
{
  read(x);
}

func R(x: dynamic) -> dynamic
{
  scanf("%d", (&x));
}

func R(x: dynamic) -> dynamic
{
  scanf("%lld", (&x));
}

func R(x: dynamic) -> dynamic
{
  scanf("%lf", (&x));
}

func R(x: dynamic) -> dynamic
{
  scanf(" %c", (&x));
}

func R(x: dynamic) -> dynamic
{
  scanf("%s", x);
}

func R() -> dynamic
{
}

func R(head: dynamic, tail: dynamic...) -> dynamic
{
  R(head);
  R(cpp_expand(tail));
}

func W(x: dynamic) -> dynamic
{
  write(x);
}

func W(x: dynamic) -> dynamic
{
  printf("%d", x);
}

func W(x: dynamic) -> dynamic
{
  printf("%lld", x);
}

func W(x: dynamic) -> dynamic
{
  printf("%.16f", x);
}

func W(x: dynamic) -> dynamic
{
  putchar(x);
}

func W(x: dynamic) -> dynamic
{
  printf("%s", x);
}

func W(x: dynamic) -> dynamic
{
  W(x.first);
  putchar(cpp_char(" "));
  W(x.second);
}

func W(x: dynamic) -> dynamic
{
  {
    var i: dynamic = x.begin();
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

func W() -> dynamic
{
}

func W(head: dynamic, tail: dynamic...) -> dynamic
{
  W(head);
  putchar( (cpp_sizeof(tail)) ? cpp_char(" ") : cpp_char("\n"));
  W(cpp_expand(tail));
}

var MOD: dynamic = (1e9 + 7);

func ADD(x: dynamic, v: dynamic) -> dynamic
{
  x = (((x + v)) % MOD);
  if ((x < 0))
  {
    x += MOD;
  }
}

var SIZE: dynamic = (1e6 + 10);

class Union_Find
{
  var d: dynamic = cpp_array(SIZE);
  var num: dynamic = cpp_array(SIZE);
  func init(n: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < (n)))
        {
          d[i] = i;
          num[i] = 1;
          i += 1;
        }
      }
    }
  func find(x: dynamic) -> dynamic
  {
      var y: dynamic = x;
      var z: dynamic = x;
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
  func is_root(x: dynamic) -> dynamic
  {
      return (d[x] == x);
    }
  func uu(x: dynamic, y: dynamic) -> dynamic
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

var U: dynamic = cpp_uninitialized();

func C2(x: dynamic) -> dynamic
{
  return ((x * ((x - 1))) / 2);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  R(n, m);
  U.init(n);
  {
    var i: dynamic = 0;
    while ((i < (m)))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      R(x, y);
      x -= 1;
      y -= 1;
      U.uu(x, y);
      i += 1;
    }
  }
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n)))
    {
      if (U.is_root(i))
      {
        res += C2(U.num[i]);
      }
      i += 1;
    }
  }
  W( ((res == m)) ? "YES" : "NO");
  return 0;
}
