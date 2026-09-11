// Translated from solution.cpp.

class Edge
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
}

var E: dynamic = cpp_array(200005);

func Cmp(a: dynamic, b: dynamic) -> dynamic
{
  return (a.v > b.v);
}

var Size: dynamic = cpp_array(200005);

var Flag: dynamic = cpp_array(200005);

var Fa: dynamic = cpp_array(200005);

func GetRoot(x: dynamic) -> dynamic
{
  return  ((x == Fa[x])) ? x : cpp_assign(Fa[x], "=", GetRoot(Fa[x]));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d%d", (&n), (&m));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%d%d", (&E[i].x), (&E[i].y), (&E[i].v));
      i += 1;
    }
  }
  sort((E + 1), ((E + m) + 1), Cmp);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      Fa[i] = i;
      Size[i] = 1;
      Flag[i] = true;
      i += 1;
    }
  }
  var Ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var Rx: dynamic = GetRoot(E[i].x);
      var Ry: dynamic = GetRoot(E[i].y);
      if (((Rx ^ Ry) && ((Flag[Rx] || Flag[Ry]))))
      {
        if ((Size[Rx] > Size[Ry]))
        {
          swap(Rx, Ry);
        }
        Fa[Rx] = Ry;
        Ans += E[i].v;
        Flag[Ry] &= Flag[Rx];
      } else if (((Rx == Ry) && Flag[Rx]))
      {
        Flag[Rx] = false;
        Ans += E[i].v;
      }
      i += 1;
    }
  }
  printf("%d\n", Ans);
  return 0;
}
