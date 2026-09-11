// Translated from solution.cpp.

var M: dynamic = 505;

class Node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var cnt: dynamic = 0;

var sum: dynamic = 0;

var k: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var book: dynamic = cpp_array(M, M);

var mapp: dynamic = cpp_array(M, M);

var mv: dynamic = [[0, -1], [0, 1], [1, 0], [-1, 0]];

func bfs(x: dynamic, y: dynamic) -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  var xt: dynamic = cpp_uninitialized();
  var yt: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
  tmp.x = x;
  tmp.y = y;
  book[x][y] = true;
  cnt = 1;
  q.push(tmp);
  while ((!q.empty()))
  {
    tmp = q.front();
    q.pop();
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        xt = (tmp.x + mv[i][0]);
        yt = (tmp.y + mv[i][1]);
        if (((((xt < 1) || (xt > n)) || (yt < 1)) || (yt > m)))
        {
          i += 1;
          continue;
        }
        if (((!book[xt][yt]) && (mapp[xt][yt] == cpp_char("."))))
        {
          if ((cnt == (sum - k)))
          {
            return;
          }
          book[xt][yt] = true;
          cnt += 1;
          next.x = xt;
          next.y = yt;
          q.push(next);
        }
        i += 1;
      }
    }
  }
}

func main() -> dynamic
{
  var x: dynamic = -1;
  var y: dynamic = -1;
  scanf("%d%d%d", (&n), (&m), (&k));
  getchar();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          scanf("%c", (&mapp[i][j]));
          if ((mapp[i][j] == cpp_char(".")))
          {
            sum += 1;
            x = i;
            y = j;
          }
          j += 1;
        }
      }
      getchar();
      i += 1;
    }
  }
  if (((x != -1) && (y != -1)))
  {
    bfs(x, y);
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
          while ((j <= m))
          {
            if (((mapp[i][j] == cpp_char(".")) && book[i][j]))
            {
              printf(".");
            } else if ((mapp[i][j] == cpp_char(".")))
            {
              printf("X");
            } else
            {
              printf("#");
            }
            j += 1;
          }
        }
        printf("\n");
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        {
          var j: dynamic = 1;
          while ((j <= m))
          {
            printf("%c", mapp[i][j]);
            j += 1;
          }
        }
        printf("\n");
        i += 1;
      }
    }
  }
  return 0;
}
