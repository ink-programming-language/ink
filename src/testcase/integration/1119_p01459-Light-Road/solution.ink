// Translated from solution.cpp.

var inf = cpp_expression("#include<i");

var X = [0, 1, 0, -1];

var Y = [-1, 0, 1, 0];

var H: dynamic;

var W: dynamic;

var A: dynamic;

var si: dynamic;

var sj: dynamic;

var map = cpp_array(100, 100);

func in_cpp(y: dynamic, x: dynamic)
{
  if (((((y < 0) || (x < 0)) || (y >= H)) || (x >= W)))
  {
    return false;
  }
  return true;
}

class State
{
  var y: dynamic;
  var x: dynamic;
  var muki: dynamic;
  var numL: dynamic;
  var numR: dynamic;
  func State(y: dynamic, x: dynamic, muki: dynamic, l: dynamic, r: dynamic)
  {
      this->y = cpp_construct(y);
      this->x = cpp_construct(x);
      this->muki = cpp_construct(muki);
      this->numL = cpp_construct(l);
      this->numR = cpp_construct(r);
    }
}

func main()
{
  var memo = cpp_array(4, 100, 100);
  {
    var i = 0;
    while ((i < 100))
    {
      {
        var j = 0;
        while ((j < 100))
        {
          {
            var k = 0;
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
  var Q: dynamic;
  read(H, W, A);
  {
    var i = 0;
    while ((i < H))
    {
      {
        var j = 0;
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
  var ans = inf;
  var a: dynamic;
  var b: dynamic;
  while ((!Q.empty()))
  {
    var u = Q.front();
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
