// Translated from solution.cpp.

var DR: dynamic = [[-1, 0], [0, 1], [1, 0], [0, -1], [-1, 1], [-1, -1], [1, 1], [1, -1]];

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if (b)
  {
    return gcd(b, (a % b));
  }
  return a;
}

var rng: dynamic = cpp_construct(chrono.steady_clock.now().time_since_epoch().count());

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var viz: dynamic = cpp_array(5005);

var match_cpp: dynamic = cpp_array(5005);

var p: dynamic = cpp_array(5005);

var c: dynamic = cpp_array(5005);

var d: dynamic = cpp_uninitialized();

var vec: dynamic = cpp_array(5005);

func f(x: dynamic) -> dynamic
{
  for (var it: dynamic in vec[x])
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cerr.tie(0);
  cout.tie(0);
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(p[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(c[i]);
      i += 1;
    }
  }
  read(d);
  var vek: dynamic = cpp_uninitialized();
  while (cpp_update(d, "--"))
  {
    var x: dynamic = cpp_uninitialized();
    read(x);
    vek.push_back(x);
  }
  reverse(vek.begin(), vek.end());
  var mex: dynamic = 0;
  var ans: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      s.insert(i);
      i += 1;
    }
  }
  for (var it: dynamic in vek)
  {
    s.erase(s.find(it));
  }
  for (var it: dynamic in s)
  {
    vec[p[it]].push_back(c[it]);
  }
  memset(match_cpp, -1, cpp_sizeof((match_cpp)));
  for (var it: dynamic in vek)
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
  for (var it: dynamic in ans)
  {
    write(it, cpp_char("\n"));
  }
}
