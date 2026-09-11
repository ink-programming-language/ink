// Translated from solution.cpp.

var N: dynamic = (2e5 + 10);

class Node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
  func operator_add(A: dynamic) -> dynamic
  {
      return [(x + A.x), (y + A.y), id];
    }
  func operator_subtract(A: dynamic) -> dynamic
  {
      return [(x - A.x), (y - A.y), id];
    }
  func operator_multiply(A: dynamic) -> dynamic
  {
      return (((1 * x) * A.y) - ((1 * y) * A.x));
    }
  func dis() -> dynamic
  {
      return (((1 * x) * x) + ((1 * y) * y));
    }
}

var A: dynamic = cpp_array(N);

var P: dynamic = cpp_array(N);

var bs: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(N);

var sta: dynamic = cpp_array(N);

var top: dynamic = cpp_uninitialized();

func cmp1(A: dynamic, B: dynamic) -> dynamic
{
  return  ((A.y != B.y)) ? (A.y < B.y) : (A.x < B.x);
}

func cmp2(A: dynamic, B: dynamic) -> dynamic
{
  return  (((A * B) == 0)) ? (A.dis() < B.dis()) : ((A * B) > 0);
}

func Convex() -> dynamic
{
  sort((A + 1), ((A + tot) + 1), cmp1);
  bs = A[1];
  top = 0;
  {
    var i: dynamic = tot;
    while ((i >= 1))
    {
      A[i] = (A[i] - A[1]);
      i -= 1;
    }
  }
  sort((A + 1), ((A + tot) + 1), cmp2);
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= top))
    {
      A[i] = A[sta[i]];
      var P: dynamic = (bs + A[i]);
      if ((((P.x & 1)) || ((P.y & 1))))
      {
        puts("Ani");
        exit(0);
      }
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d%d", (&P[i].x), (&P[i].y));
      P[i].id = i;
      i += 1;
    }
  }
  P[cpp_update(n, "++")] = [0, 0, 0];
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
    while ((i <= top))
    {
      vis[A[i].id] =  (((i & 1))) ? 1 : 2;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
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
