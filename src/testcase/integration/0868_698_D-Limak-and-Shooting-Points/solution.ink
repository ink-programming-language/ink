// Translated from solution.cpp.

func read(x: dynamic)
{
  x = 0;
  var tmp: dynamic;
  var key = 0;
  {
    tmp = getchar();
    while ((!isdigit(tmp)))
    {
      key = ((tmp == cpp_char("-")));
      tmp = getchar();
    }
  }
  {
    while (isdigit(tmp))
    {
      x = ((((x << 3)) + ((x << 1))) + ((tmp ^ cpp_char("0"))));
      tmp = getchar();
    }
  }
  if (key)
  {
    x = (-x);
  }
}

func ckmn(x: dynamic, y: dynamic)
{
  x = if ((x < y)) x else y;
}

func ckmx(x: dynamic, y: dynamic)
{
  x = if ((x < y)) y else x;
}

class point
{
  var x: dynamic;
  var y: dynamic;
  func point(x: dynamic = 0, y: dynamic = 0)
  {
      this->x = cpp_construct(x);
      this->y = cpp_construct(y);
    }
  func operator_add(a: dynamic)
  {
      return point((x + a.x), (y + a.y));
    }
  func operator_subtract(a: dynamic)
  {
      return point((x - a.x), (y - a.y));
    }
}

func cross(a: dynamic, b: dynamic)
{
  return (((1 * a.x) * b.y) - ((1 * a.y) * b.x));
}

func dot(a: dynamic, b: dynamic)
{
  return (((1 * a.x) * b.x) + ((1 * a.y) * b.y));
}

var N = 1010;

var K = 10;

var n: dynamic;

var k: dynamic;

var ans: dynamic;

var per = cpp_array(K);

var tmp: dynamic;

var vis = cpp_array(N);

var rec: dynamic;

var p1 = cpp_array(N);

var p2 = cpp_array(N);

var pat = cpp_array(N, K);

func dfs(cur: dynamic)
{
  if ((tmp >= k))
  {
    return false;
  }
  var x = per[cpp_update(tmp, "++")];
  {
    var i = (0);
    while ((i <= ((cpp_cast(pat[x][cur].size()) - 1))))
    {
      if ((vis[pat[x][cur][i]] != rec))
      {
        if ((!dfs(pat[x][cur][i])))
        {
          return false;
        }
      }
      i += 1;
    }
  }
  vis[cur] = rec;
  return true;
}

func main()
{
  read(k);
  read(n);
  {
    var i = (1);
    while ((i <= (k)))
    {
      read(p1[i].x);
      read(p1[i].y);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (n)))
    {
      read(p2[i].x);
      read(p2[i].y);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (k)))
    {
      {
        var a = (1);
        while ((a <= (n)))
        {
          {
            var b = (1);
            while ((b <= (n)))
            {
              if ((cross((p2[b] - p1[i]), (p2[a] - p1[i])) == 0))
              {
                if ((dot((p1[i] - p2[b]), (p2[a] - p2[b])) < 0))
                {
                  pat[i][a].push_back(b);
                }
              }
              b += 1;
            }
          }
          a += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i <= (n)))
    {
      var key = 0;
      {
        var j = (1);
        while ((j <= (k)))
        {
          per[j] = j;
          j += 1;
        }
      }
      while (true)
      {
        tmp = 0;
        rec += 1;
        if (dfs(i))
        {
          key = 1;
          break;
        }
        if (!((next_permutation((per + 1), ((per + k) + 1)))))
        {
          break;
        }
      }
      ans += key;
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
