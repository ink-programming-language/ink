// Translated from solution.cpp.

func ni()
{
  var val: dynamic;
  scanf("%i", (&val));
  return val;
}

func npi()
{
  var val: dynamic;
  scanf("%i %i", (&val.first), (&val.second));
  return val;
}

func nll()
{
  var val: dynamic;
  scanf("%I64d", (&val));
  return val;
}

func nvi(n: dynamic, corr: dynamic = 0)
{
  {
    var i = 0;
    while ((i < n))
    {
      a[i] = (ni() + corr);
      i += 1;
    }
  }
  return move(a);
}

func nc()
{
  var val: dynamic;
  while (true)
  {
    val = getchar();
    if (!(((((val == cpp_char(" ")) || (val == cpp_char("\r"))) || (val == cpp_char("\n"))))))
    {
      break;
    }
  }
  return val;
}

func ncs()
{
  var val: dynamic;
  while (true)
  {
    val = getchar();
    if (!((false)))
    {
      break;
    }
  }
  return val;
}

func ns()
{
  var buff = cpp_array((1024 * 4000));
  scanf("%s", buff);
  return [buff];
}

func gcd(a: dynamic, b: dynamic)
{
  while (b)
  {
    var tmp = (a % b);
    a = b;
    b = tmp;
  }
  return a;
}

func tr2(xv1: dynamic, yv1: dynamic, xv2: dynamic, yv2: dynamic, x3: dynamic, y3: dynamic)
{
  return (((1 * ((xv2 - xv1))) * ((y3 - yv1))) - ((1 * ((yv2 - yv1))) * ((x3 - xv1))));
}

var eps = 1e-12;

var pi = acos(-1.0);

func eq(a: dynamic, b: dynamic)
{
  return (abs((a - b)) <= eps);
}

var bits_cnt = cpp_array(256);

var input_dir = "inputs\\";

var input_file = (input_dir + "input.txt");

var output_file = (input_dir + "output.txt");

func init_streams()
{
}

func init_data()
{
  {
    var i = 1;
    while ((i <= 255))
    {
      {
        var j = 0;
        while ((j < 8))
        {
          if ((((1 << j)) & i))
          {
            bits_cnt[i] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func bit_count(v: dynamic)
{
  return (((bits_cnt[(v & 0xFF)] + bits_cnt[(((v >> 8)) & 0xFF)]) + bits_cnt[(((v >> 16)) & 0xFF)]) + bits_cnt[(((v >> 24)) & 0xFF)]);
}

var maxn = (100000 + 1);

var n: dynamic;

var m: dynamic;

var E = cpp_array(maxn);

var adj = cpp_array(maxn);

var vis = cpp_array(maxn);

var mx = cpp_array(maxn);

func dfs(u: dynamic)
{
  if (vis[u])
  {
    return mx[u];
  }
  vis[u] = true;
  mx[u] = E[u];
  for (var to in adj[u])
  {
    mx[u] = max(mx[u], (((E[u] && (!E[to]))) + dfs(to)));
  }
  return mx[u];
}

func main()
{
  init_streams();
  init_data();
  var n = ni();
  var m = ni();
  {
    var i = 0;
    while ((i < n))
    {
      E[i] = ni();
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var u = ni();
      var v = ni();
      adj[u].emplace_back(v);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < n))
    {
      ans = max(ans, dfs(i));
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
