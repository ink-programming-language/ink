// Translated from solution.cpp.

var DR = [[-1, 0], [0, 1], [1, 0], [0, -1], [-1, 1], [-1, -1], [1, 1], [1, -1]];

func gcd(a: dynamic, b: dynamic)
{
  if (b)
  {
    return gcd(b, (a % b));
  }
  return a;
}

var rng = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic;

var m: dynamic;

var viz = cpp_array(5005);

var match_cpp = cpp_array(5005);

var p = cpp_array(5005);

var c = cpp_array(5005);

var d: dynamic;

var vec = cpp_array(5005);

func f(x: dynamic)
{
  for (var it in vec[x])
  {
    if ((!viz[it]))
    {
      viz[it] = 1;
      if (((match_cpp[it] == -1) || f(match_cpp[it])))
      {
        match_cpp[it] = x;
        return 1;
      }
    }
  }
  return 0;
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cerr.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      read(p[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      read(c[i]);
      i += 1;
    }
  }
  read(d);
  var vek: dynamic;
  while (cpp_update(d, "--"))
  {
    var x: dynamic;
    read(x);
    vek.push_back(x);
  }
  reverse(vek.begin(), vek.end());
  var mex = 0;
  var ans: dynamic;
  var s: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      s.insert(i);
      i += 1;
    }
  }
  for (var it in vek)
  {
    s.erase(s.find(it));
  }
  for (var it in s)
  {
    vec[p[it]].push_back(c[it]);
  }
  memset(match_cpp, -1, cpp_sizeof((match_cpp)));
  for (var it in vek)
  {
    while (1)
    {
      memset(viz, 0, cpp_sizeof((viz)));
      if (f(mex))
      {
        mex += 1;
      } else
      {
        break;
      }
    }
    ans.push_back(mex);
    vec[p[it]].push_back(c[it]);
  }
  reverse(ans.begin(), ans.end());
  for (var it in ans)
  {
    write(it, cpp_char("\n"));
  }
}
