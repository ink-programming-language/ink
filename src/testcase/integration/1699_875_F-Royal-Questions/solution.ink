// Translated from solution.cpp.

class Edge
{
  var x: dynamic;
  var y: dynamic;
  var v: dynamic;
}

var E = cpp_array(200005);

func Cmp(a: dynamic, b: dynamic)
{
  return (a.v > b.v);
}

var Size = cpp_array(200005);

var Flag = cpp_array(200005);

var Fa = cpp_array(200005);

func GetRoot(x: dynamic)
{
  return if ((x == Fa[x])) x else cpp_assign(Fa[x], "=", GetRoot(Fa[x]));
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= m))
    {
      scanf("%d%d%d", (&E[i].x), (&E[i].y), (&E[i].v));
      i += 1;
    }
  }
  sort((E + 1), ((E + m) + 1), Cmp);
  {
    var i = 1;
    while ((i <= n))
    {
      Fa[i] = i;
      Size[i] = 1;
      Flag[i] = true;
      i += 1;
    }
  }
  var Ans = 0;
  {
    var i = 1;
    while ((i <= m))
    {
      var Rx = GetRoot(E[i].x);
      var Ry = GetRoot(E[i].y);
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
