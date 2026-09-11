// Translated from solution.cpp.

var MAXN: dynamic = 55;

var MAXM: dynamic = 55;

var MAXS: dynamic = 11111;

var N: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var Cnt: dynamic = cpp_uninitialized();

var Inv: dynamic = false;

class Pos
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  func Pos() -> dynamic
  {
    }
  func Pos(x: dynamic, y: dynamic) -> dynamic
  {
      x = x;
      y = y;
    }
  func read() -> dynamic
  {
      scanf("%d%d", (&x), (&y));
    }
}

func operator_equal(A: dynamic, B: dynamic) -> dynamic
{
  return ((A.x == B.x) && (A.y == B.y));
}

func cmpp(A: dynamic, B: dynamic) -> dynamic
{
  if ((A.x == B.x))
  {
    if (((A.x + B.x) == 2))
    {
      return (A.y < B.y);
    }
    return (A.y > B.y);
  }
  return (A.x < B.x);
}

class Move
{
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  func Move() -> dynamic
  {
    }
  func Move(a: dynamic, b: dynamic) -> dynamic
  {
      A = a;
      B = b;
    }
  func len() -> dynamic
  {
      return (abs((A.x - B.x)) + abs((A.y - B.y)));
    }
  func zig() -> dynamic
  {
      swap(A, B);
    }
  func show() -> dynamic
  {
      if (Inv)
      {
        A.x = ((N + 1) - A.x);
        B.x = ((N + 1) - B.x);
      }
      if ((A.x == B.x))
      {
        if ((A.y < B.y))
        {
          {
            var i: dynamic = A.y;
            while ((i < B.y))
            {
              printf("%d %d %d %d\n", A.x, i, A.x, (i + 1));
              i += 1;
            }
          }
        }
        if ((A.y > B.y))
        {
          {
            var i: dynamic = A.y;
            while ((i > B.y))
            {
              printf("%d %d %d %d\n", A.x, i, A.x, (i - 1));
              i -= 1;
            }
          }
        }
      }
      if ((A.y == B.y))
      {
        if ((A.x < B.x))
        {
          {
            var i: dynamic = A.x;
            while ((i < B.x))
            {
              printf("%d %d %d %d\n", i, A.y, (i + 1), A.y);
              i += 1;
            }
          }
        }
        if ((A.x > B.x))
        {
          {
            var i: dynamic = A.x;
            while ((i > B.x))
            {
              printf("%d %d %d %d\n", i, A.y, (i - 1), A.y);
              i -= 1;
            }
          }
        }
      }
    }
}

var P: dynamic = cpp_array(MAXS, 2);

var Pc: dynamic = cpp_array(2);

func show() -> dynamic
{
  Cnt = 0;
  {
    var i: dynamic = 1;
    while ((i <= Pc[0]))
    {
      Cnt += P[0][i].len();
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= Pc[1]))
    {
      Cnt += P[1][i].len();
      i += 1;
    }
  }
  printf("%d\n", Cnt);
  {
    var i: dynamic = 1;
    while ((i <= Pc[0]))
    {
      P[0][i].show();
      i += 1;
    }
  }
  {
    var i: dynamic = Pc[1];
    while ((i >= 1))
    {
      P[1][i].zig();
      P[1][i].show();
      i -= 1;
    }
  }
}

func Push(A: dynamic, B: dynamic, d: dynamic) -> dynamic
{
  if ((A == B))
  {
    return;
  }
  assert(((A.x == B.x) || (A.y == B.y)));
  P[d][cpp_update(Pc[d], "++")] = Move(A, B);
}

func Jump(A: dynamic, B: dynamic, d: dynamic) -> dynamic
{
  if ((A == B))
  {
    return;
  }
  var Temp: dynamic = cpp_uninitialized();
  if ((A.y > B.y))
  {
    Temp = Pos(B.x, A.y);
  } else
  {
    Temp = Pos(A.x, B.y);
  }
  Push(A, Temp, d);
  Push(Temp, B, d);
}

func Jump_zig(A: dynamic, B: dynamic, d: dynamic) -> dynamic
{
  if ((A.y == B.y))
  {
    Push(A, B, d);
    return;
  }
  var Temp1: dynamic = cpp_uninitialized();
  var Temp2: dynamic = cpp_uninitialized();
  Temp1 = Pos((A.x - 1), A.y);
  Temp2 = Pos((B.x + 1), B.y);
  Push(A, Temp1, d);
  Push(Temp1, Temp2, d);
  Push(Temp2, B, d);
}

class Cube
{
  var P: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var S: dynamic = cpp_array(MAXM);

var T: dynamic = cpp_array(MAXM);

func operator_less(A: dynamic, B: dynamic) -> dynamic
{
  return cmpp(A.P, B.P);
}

func main() -> dynamic
{
  scanf("%d%d", (&N), (&M));
  {
    var i: dynamic = 1;
    while ((i <= M))
    {
      S[i].P.read();
      S[i].id = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= M))
    {
      T[i].P.read();
      T[i].id = i;
      i += 1;
    }
  }
  if ((M == 1))
  {
    Jump(S[1].P, T[1].P, 0);
    show();
    return 0;
  }
  sort((S + 1), ((S + M) + 1));
  {
    var i: dynamic = 1;
    while ((i <= M))
    {
      Jump(S[i].P, Pos(1, i), 0);
      i += 1;
    }
  }
  if ((N == 2))
  {
    if ((S[1].id > S[2].id))
    {
      Push(Pos(1, 2), Pos(2, 2), 0);
      Push(Pos(1, 1), Pos(1, 2), 0);
      Push(Pos(2, 2), Pos(2, 1), 0);
      Push(Pos(2, 1), Pos(1, 1), 0);
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= M))
      {
        Push(Pos(1, i), Pos(3, i), 0);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= M))
      {
        Jump_zig(Pos(3, i), Pos(1, S[i].id), 0);
        i += 1;
      }
    }
  }
  sort((T + 1), ((T + M) + 1));
  {
    var i: dynamic = 1;
    while ((i <= M))
    {
      Jump(T[i].P, Pos(1, i), 1);
      i += 1;
    }
  }
  if ((N == 2))
  {
    if ((T[1].id > T[2].id))
    {
      Push(Pos(1, 2), Pos(2, 2), 1);
      Push(Pos(1, 1), Pos(1, 2), 1);
      Push(Pos(2, 2), Pos(2, 1), 1);
      Push(Pos(2, 1), Pos(1, 1), 1);
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= M))
      {
        Push(Pos(1, i), Pos(3, i), 1);
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= M))
      {
        Jump_zig(Pos(3, i), Pos(1, T[i].id), 1);
        i += 1;
      }
    }
  }
  show();
  return 0;
}
