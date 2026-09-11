// Translated from solution.cpp.

var PII: dynamic = cpp_expression("#include<cstd");

var pb: dynamic = cpp_expression("#include<");

var ep: dynamic = cpp_expression("#include<cst");

var mp: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 3)) + ((x << 1))) + ((c ^ 48)));
    c = getchar();
  }
  return  ((f == 1)) ? x : ((~x) + 1);
}

func print(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = ((~x) + 1);
  }
  if ((x >= 10))
  {
    print((x / 10));
  }
  putchar((((x % 10)) | 48));
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var col: dynamic = cpp_array(210);

var dis: dynamic = cpp_array(210);

var cnt: dynamic = cpp_array(210);

var inq: dynamic = cpp_array(210);

var vec: dynamic = cpp_array(210);

func fail() -> dynamic
{
  printf("No\n");
  exit(0);
}

func bfs() -> dynamic
{
  memset(col, -1, cpp_sizeof((col)));
  var Q: dynamic = cpp_uninitialized();
  Q.push(1);
  col[1] = 0;
  while ((!Q.empty()))
  {
    var u: dynamic = Q.front();
    Q.pop();
    for (var i: dynamic in vec[u])
    {
      var v: dynamic = i.fi;
      if ((col[v] == -1))
      {
        col[v] = (col[u] ^ 1);
        Q.push(v);
      } else if ((col[v] != ((col[u] ^ 1))))
      {
        fail();
      }
    }
  }
}

func spfa(s: dynamic) -> dynamic
{
  memset(cnt, 0, cpp_sizeof((cnt)));
  memset(dis, 0x3f, cpp_sizeof((dis)));
  var Q: dynamic = cpp_uninitialized();
  Q.push(s);
  dis[s] = 0;
  cnt[s] = 1;
  inq[s] = 1;
  while ((!Q.empty()))
  {
    var u: dynamic = Q.front();
    Q.pop();
    inq[u] = 0;
    for (var i: dynamic in vec[u])
    {
      var v: dynamic = i.fi;
      var d: dynamic = (dis[u] + i.se);
      if ((dis[v] > d))
      {
        dis[v] = d;
        cnt[v] += 1;
        if ((cnt[v] >= n))
        {
          fail();
        }
        if ((!inq[v]))
        {
          Q.push(v);
          inq[v] = 1;
        }
      }
    }
  }
}

var f: dynamic = cpp_array(210, 210);

func main() -> dynamic
{
  n = read();
  m = read();
  memset(f, 0x15, cpp_sizeof((f)));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      var u: dynamic = read();
      var v: dynamic = read();
      var b: dynamic = read();
      if (b)
      {
        vec[u].ep(v, -1);
        f[u][v] = -1;
      } else
      {
        vec[u].ep(v, 1);
        f[u][v] = 1;
      }
      vec[v].ep(u, 1);
      f[v][u] = 1;
      i += 1;
    }
  }
  bfs();
  spfa(1);
  {
    var k: dynamic = 1;
    while ((k <= n))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          {
            var j: dynamic = 1;
            while ((j <= n))
            {
              if ((((i == j) || (i == k)) || (j == k)))
              {
                j += 1;
                continue;
              }
              f[i][j] = min(f[i][j], (f[i][k] + f[k][j]));
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  var mxn: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((i == j))
          {
            j += 1;
            continue;
          } else
          {
            mxn = max(mxn, abs(f[i][j]));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("YES\n");
  printf("%d\n", mxn);
  var flag: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= n))
        {
          if ((f[i][j] == mxn))
          {
            flag = 1;
            spfa(i);
            break;
          }
          j += 1;
        }
      }
      if (flag)
      {
        break;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      printf("%d ", ((-dis[i]) + 1000));
      i += 1;
    }
  }
  return 0;
}
