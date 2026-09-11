// Translated from solution.cpp.

var inf: dynamic = cpp_expression("#include<i");

var X: dynamic = [0, 1, 0, -1];

var Y: dynamic = [-1, 0, 1, 0];

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var A: dynamic = cpp_uninitialized();

var si: dynamic = cpp_uninitialized();

var sj: dynamic = cpp_uninitialized();

var map: dynamic = cpp_array(100, 100);

func in_cpp(y: dynamic, x: dynamic) -> dynamic
{
  if (((((y < 0) || (x < 0)) || (y >= H)) || (x >= W)))
  {
    return false;
  }
  return true;
}

class State
{
  var y: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var muki: dynamic = cpp_uninitialized();
  var numL: dynamic = cpp_uninitialized();
  var numR: dynamic = cpp_uninitialized();
  func State(y: dynamic, x: dynamic, muki: dynamic, l: dynamic, r: dynamic) -> dynamic
  {
      self->y = cpp_construct(y);
      self->x = cpp_construct(x);
      self->muki = cpp_construct(muki);
      self->numL = cpp_construct(l);
      self->numR = cpp_construct(r);
    }
}

func main() -> dynamic
{
  var memo: dynamic = cpp_array(4, 100, 100);
  {
    var i: dynamic = 0;
    while ((i < 100))
    {
      {
        var j: dynamic = 0;
        while ((j < 100))
        {
          {
            var k: dynamic = 0;
            while ((k < 4))
            {
              memo[i][j][k] = inf;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var Q: dynamic = cpp_uninitialized();
  read(H, W, A);
  {
    var i: dynamic = 0;
    while ((i < H))
    {
      {
        var j: dynamic = 0;
        while ((j < W))
        {
          read(map[i][j]);
          if ((map[i][j] == cpp_char("S")))
          {
            si = i;
            sj = j;
            map[i][j] = cpp_char(".");
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  Q.push(State(si, sj, 2, 0, 0));
  memo[si][sj][0] = 0;
  memo[si][sj][2] = 0;
  var ans: dynamic = inf;
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  while ((!Q.empty()))
  {
    var u: dynamic = Q.front();
    Q.pop();
    if (((u.numL > A) || (u.numR > A)))
    {
      continue;
    }
    if ((map[u.y][u.x] == cpp_char("G")))
    {
      ans = min(memo[u.y][u.x][u.muki], ans);
      continue;
    }
    a = (u.y + Y[u.muki]);
    b = (u.x + X[u.muki]);
    if (in_cpp(a, b))
    {
      if ((map[a][b] != cpp_char("#")))
      {
        if ((memo[a][b][u.muki] >= (u.numL + u.numR)))
        {
          memo[a][b][u.muki] = (u.numL + u.numR);
          Q.push(State(a, b, u.muki, u.numL, u.numR));
        }
      }
    }
    if (((u.y == si) && (u.x == sj)))
    {
    } else
    {
      a = (u.y + Y[(((u.muki + 1)) % 4)]);
      b = (u.x + X[(((u.muki + 1)) % 4)]);
      if (in_cpp(a, b))
      {
        if ((map[a][b] != cpp_char("#")))
        {
          if ((memo[a][b][(((u.muki + 1)) % 4)] >= ((u.numL + u.numR) + 1)))
          {
            memo[a][b][(((u.muki + 1)) % 4)] = ((u.numR + u.numL) + 1);
            if ((memo[a][b][(((u.muki + 1)) % 4)] < ans))
            {
              if (((((((u.muki + 1)) % 4)) % 2) == 0))
              {
                if ((u.numL < A))
                {
                  Q.push(State(a, b, (((u.muki + 1)) % 4), (u.numL + 1), u.numR));
                }
              }
              if (((((((u.muki + 1)) % 4)) % 2) == 1))
              {
                if ((u.numR < A))
                {
                  Q.push(State(a, b, (((u.muki + 1)) % 4), u.numL, (u.numR + 1)));
                }
              }
            }
          }
        }
      }
      a = (u.y + Y[(((u.muki + 3)) % 4)]);
      b = (u.x + X[(((u.muki + 3)) % 4)]);
      if (in_cpp(a, b))
      {
        if ((map[a][b] != cpp_char("#")))
        {
          if ((memo[a][b][(((u.muki + 3)) % 4)] >= ((u.numR + u.numL) + 1)))
          {
            memo[a][b][(((u.muki + 3)) % 4)] = ((u.numR + u.numL) + 1);
            if ((memo[a][b][(((u.muki + 3)) % 4)] < ans))
            {
              if (((((((u.muki + 3)) % 4)) % 2) == 0))
              {
                if ((u.numR < A))
                {
                  Q.push(State(a, b, (((u.muki + 3)) % 4), u.numL, (u.numR + 1)));
                }
              }
              if (((((((u.muki + 3)) % 4)) % 2) == 1))
              {
                if ((u.numL < A))
                {
                  Q.push(State(a, b, (((u.muki + 3)) % 4), (u.numL + 1), u.numR));
                }
              }
            }
          }
        }
      }
    }
  }
  if ((ans == inf))
  {
    write(-1, "\n");
  } else
  {
    write(ans, "\n");
  }
  return 0;
}
