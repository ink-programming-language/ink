// Translated from solution.cpp.

var ll: dynamic = dynamic;

var ull: dynamic = dynamic;

var pii: dynamic = cpp_expression("// Author: wlz");

var pb: dynamic = cpp_expression("// Author");

var fir: dynamic = cpp_expression("// Au");

var sec: dynamic = cpp_expression("// Aut");

func rep(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  cpp_macro("for (int i = l; i <= r; i++)");
}

func per(i: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  cpp_macro("for (int i = l; i >= r; i--)");
}

func mset(s: dynamic, t: dynamic) -> dynamic
{
  return cpp_expression("// Author: wlzhouzhuan");
}

func mcpy(s: dynamic, t: dynamic) -> dynamic
{
  return cpp_expression("// Author: wlzhouzhuan");
}

var poly: dynamic = cpp_expression("// Author:");

func SZ(x: dynamic) -> dynamic
{
  return cpp_expression("// Author: wlzh");
}

func ckmin(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

func ckmax(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
  }
}

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 0;
  var ch: dynamic = getchar();
  while ((!isdigit(ch)))
  {
    f |= (ch == cpp_char("-"));
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((10 * x) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return  (f) ? (-x) : x;
}

func print(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  if ((x >= 10))
  {
    print((x / 10));
  }
  putchar(((x % 10) + cpp_char("0")));
}

func print(x: dynamic, let_cpp: dynamic) -> dynamic
{
  print(x);
  putchar(let_cpp);
}

var lg: dynamic = cpp_array(20005);

var Max: dynamic = cpp_array(15, 20005);

var r: dynamic = cpp_array(20005);

var f: dynamic = cpp_array(15, 31, 20005);

var n: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func MAX(x: dynamic, y: dynamic) -> dynamic
{
  return  ((r[x] > r[y])) ? x : y;
}

func qmax(l: dynamic, r: dynamic) -> dynamic
{
  var len: dynamic = lg[((r - l) + 1)];
  return MAX(Max[l][len], Max[((r - ((1 << len))) + 1)][len]);
}

func main() -> dynamic
{
  n = read();
  q = read();
  m = 30;
  lg[1] = 0;
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      lg[i] = (lg[(i >> 1)] + 1);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      r[i] = min((i + read()), n);
      Max[i][0] = i;
      i += 1;
    }
  }
  {
    var j: dynamic = 1;
    while ((j < 15))
    {
      {
        var i: dynamic = 1;
        while ((((i + ((1 << j))) - 1) <= n))
        {
          Max[i][j] = MAX(Max[i][(j - 1)], Max[(i + ((1 << (j - 1))))][(j - 1)]);
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j <= m))
        {
          f[i][j][0] = min((r[i] + j), n);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var k: dynamic = 1;
    while ((k < 15))
    {
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          {
            var j: dynamic = 0;
            while ((j <= m))
            {
              {
                var l: dynamic = 0;
                while ((l <= j))
                {
                  var who: dynamic = qmax(i, f[i][l][(k - 1)]);
                  ckmax(f[i][j][k], f[who][(j - l)][(k - 1)]);
                  l += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      k += 1;
    }
  }
  while (cpp_update(q, "--"))
  {
    var L: dynamic = read();
    var R: dynamic = read();
    var k: dynamic = read();
    if ((L == R))
    {
      puts("0");
      continue;
    }
    var far: dynamic = cpp_array(31);
    var tmp: dynamic = cpp_array(31);
    var ans: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i <= k))
      {
        far[i] = L;
        i += 1;
      }
    }
    {
      var i: dynamic = 14;
      while ((i >= 0))
      {
        var ok: dynamic = 0;
        {
          var j: dynamic = 0;
          while ((j <= k))
          {
            tmp[j] = 0;
            {
              var l: dynamic = 0;
              while ((l <= j))
              {
                var who: dynamic = qmax(L, far[l]);
                ckmax(tmp[j], f[who][(j - l)][i]);
                l += 1;
              }
            }
            if ((tmp[j] >= R))
            {
              ok = 1;
              break;
            }
            j += 1;
          }
        }
        if ((!ok))
        {
          ans += (1 << i);
          memcpy(far, tmp, (4 * ((k + 1))));
        }
        i -= 1;
      }
    }
    print((ans + 1), cpp_char("\n"));
  }
  return 0;
}
