// Translated from solution.cpp.

func ni() -> dynamic
{
  var val: dynamic = cpp_uninitialized();
  scanf("%i", (&val));
  return val;
}

func npi() -> dynamic
{
  var val: dynamic = cpp_uninitialized();
  scanf("%i %i", (&val.first), (&val.second));
  return val;
}

func nll() -> dynamic
{
  var val: dynamic = cpp_uninitialized();
  scanf("%I64d", (&val));
  return val;
}

func nvi(n: dynamic, corr: dynamic = 0) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      a[i] = (ni() + corr);
      i += 1;
    }
  }
  return move(a);
}

func nc() -> dynamic
{
  var val: dynamic = cpp_uninitialized();
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

func ncs() -> dynamic
{
  var val: dynamic = cpp_uninitialized();
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

func ns() -> dynamic
{
  var buff: dynamic = cpp_array((1024 * 4000));
  scanf("%s", buff);
  return [buff];
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  while (b)
  {
    var tmp: dynamic = (a % b);
    a = b;
    b = tmp;
  }
  return a;
}

func tr2(xv1: dynamic, yv1: dynamic, xv2: dynamic, yv2: dynamic, x3: dynamic, y3: dynamic) -> dynamic
{
  return (((1 * ((xv2 - xv1))) * ((y3 - yv1))) - ((1 * ((yv2 - yv1))) * ((x3 - xv1))));
}

var eps: dynamic = 1e-12;

var pi: dynamic = acos(-1.0);

func eq(a: dynamic, b: dynamic) -> dynamic
{
  return (abs((a - b)) <= eps);
}

var bits_cnt: dynamic = cpp_array(256);

var input_dir: dynamic = "inputs\\";

var input_file: dynamic = (input_dir + "input.txt");

var output_file: dynamic = (input_dir + "output.txt");

func init_streams() -> dynamic
{
}

func init_data() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= 255))
    {
      {
        var j: dynamic = 0;
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

func bit_count(v: dynamic) -> dynamic
{
  return (((bits_cnt[(v & 0xFF)] + bits_cnt[(((v >> 8)) & 0xFF)]) + bits_cnt[(((v >> 16)) & 0xFF)]) + bits_cnt[(((v >> 24)) & 0xFF)]);
}

var maxn: dynamic = (100000 + 1);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var E: dynamic = cpp_array(maxn);

var adj: dynamic = cpp_array(maxn);

var vis: dynamic = cpp_array(maxn);

var mx: dynamic = cpp_array(maxn);

func dfs(u: dynamic) -> dynamic
{
  if (vis[u])
  {
    return mx[u];
  }
  vis[u] = true;
  mx[u] = E[u];
  for (var to: dynamic in adj[u])
  {
    mx[u] = max(mx[u], (((E[u] && (!E[to]))) + dfs(to)));
  }
  return mx[u];
}

func main() -> dynamic
{
  init_streams();
  init_data();
  var n: dynamic = ni();
  var m: dynamic = ni();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      E[i] = ni();
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var u: dynamic = ni();
      var v: dynamic = ni();
      adj[u].emplace_back(v);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      ans = max(ans, dfs(i));
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
