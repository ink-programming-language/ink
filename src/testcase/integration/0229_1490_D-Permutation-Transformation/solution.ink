// Translated from solution.cpp.

var N: dynamic = cpp_expression("//firstly");

var M: dynamic = cpp_expression("//firstly");

var ll: dynamic = dynamic;

var pll: dynamic = cpp_expression("//firstly s");

var vll: dynamic = cpp_expression("//firstly");

var vpll: dynamic = cpp_expression("//firstly s");

var vvll: dynamic = cpp_expression("//firstly s");

var endl: dynamic = cpp_expression("//fi");

var umap: dynamic = cpp_expression("//firstly save by ct");

var adj: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_uninitialized();

var viss: dynamic = cpp_uninitialized();

var rnk: dynamic = cpp_uninitialized();

var parent: dynamic = cpp_uninitialized();

var sz: dynamic = cpp_uninitialized();

var sieve_max: dynamic = cpp_expression("//first");

var spf: dynamic = cpp_array((sieve_max + 1));

var mp: dynamic = cpp_uninitialized();

func yg(v: dynamic, l: dynamic, r: dynamic, prev: dynamic) -> dynamic
{
  if ((r < l))
  {
    return;
  }
  if ((r == l))
  {
    mp[r] = prev;
    return;
  }
  var mx: dynamic = LLONG_MIN;
  var ct: dynamic = 0;
  {
    var i: dynamic = l;
    while ((i <= r))
    {
      if ((v[i] > mx))
      {
        mx = v[i];
        ct = i;
      }
      i += 1;
    }
  }
  mp[ct] = prev;
  yg(v, l, (ct - 1), (prev + 1));
  yg(v, (ct + 1), r, (prev + 1));
  return;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    mp.clear();
    var n: dynamic = cpp_uninitialized();
    read(n);
    var v: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(temp);
        v.push_back(temp);
        i += 1;
      }
    }
    yg(v, 0, (n - 1), 0);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        write(mp[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
