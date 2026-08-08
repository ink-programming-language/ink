// Translated from solution.cpp.

var N = (2e5 + 10);

class Node
{
  var x: dynamic;
  var y: dynamic;
  var id: dynamic;
  func operator_add(A: dynamic)
  {
      return [(x + A.x), (y + A.y), id];
    }
  func operator_subtract(A: dynamic)
  {
      return [(x - A.x), (y - A.y), id];
    }
  func operator_multiply(A: dynamic)
  {
      return (((1 * x) * A.y) - ((1 * y) * A.x));
    }
  func dis()
  {
      return (((1 * x) * x) + ((1 * y) * y));
    }
}

var A = cpp_array(N);

var P = cpp_array(N);

var bs: dynamic;

var n: dynamic;

var tot: dynamic;

var vis = cpp_array(N);

var sta = cpp_array(N);

var top: dynamic;

func cmp1(A: dynamic, B: dynamic)
{
  return if ((A.y != B.y)) (A.y < B.y) else (A.x < B.x);
}

func cmp2(A: dynamic, B: dynamic)
{
  return if (((A * B) == 0)) (A.dis() < B.dis()) else ((A * B) > 0);
}

func Convex()
{
  sort((A + 1), ((A + tot) + 1), cmp1);
  bs = A[1];
  top = 0;
  {
    var i = tot;
    while ((i >= 1))
    {
      A[i] = (A[i] - A[1]);
      i -= 1;
    }
  }
  sort((A + 1), ((A + tot) + 1), cmp2);
  {
    var i = 1;
    while ((i <= tot))
    {
      while (((top >= 2) && ((((A[i] - A[sta[(top - 1)]])) * ((A[sta[top]] - A[sta[(top - 1)]]))) >= 0)))
      {
        top -= 1;
      }
      sta[cpp_update(top, "++")] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= top))
    {
      A[i] = A[sta[i]];
      var P = (bs + A[i]);
      if ((((P.x & 1)) || ((P.y & 1))))
      {
        puts("Ani");
        exit(0);
      }
      i += 1;
    }
  }
}

func main()
{
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&P[i].x), (&P[i].y));
      P[i].id = i;
      i += 1;
    }
  }
  P[cpp_update(n, "++")] = [0, 0, 0];
  {
    var i = 1;
    while ((i <= n))
    {
      A[i] = P[i];
      i += 1;
    }
  }
  tot = n;
  Convex();
  tot = 0;
  {
    var i = 1;
    while ((i <= top))
    {
      vis[A[i].id] = if (((i & 1))) 1 else 2;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((vis[i] != 1))
      {
        A[cpp_update(tot, "++")] = P[i];
      }
      i += 1;
    }
  }
  Convex();
  tot = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((vis[i] != 2))
      {
        A[cpp_update(tot, "++")] = P[i];
      }
      i += 1;
    }
  }
  Convex();
  puts("Borna");
}
