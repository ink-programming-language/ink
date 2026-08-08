// Translated from solution.cpp.

var M = 505;

class Node
{
  var x: dynamic;
  var y: dynamic;
}

var cnt = 0;

var sum = 0;

var k: dynamic;

var n: dynamic;

var m: dynamic;

var book = cpp_array(M, M);

var mapp = cpp_array(M, M);

var mv = [[0, -1], [0, 1], [1, 0], [-1, 0]];

func bfs(x: dynamic, y: dynamic)
{
  var q: dynamic;
  var xt: dynamic;
  var yt: dynamic;
  var tmp: dynamic;
  var next: dynamic;
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
      var i = 0;
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

func main()
{
  var x = -1;
  var y = -1;
  scanf("%d%d%d", (&n), (&m), (&k));
  getchar();
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
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
      var i = 1;
      while ((i <= n))
      {
        {
          var j = 1;
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
