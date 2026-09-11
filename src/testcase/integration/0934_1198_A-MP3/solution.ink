// Translated from solution.cpp.

var N: dynamic = (4e5 + 5);

var a: dynamic = cpp_array(N);

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var I: dynamic = cpp_uninitialized();
  read(n, I);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  var k: dynamic = (((8 * I)) / n);
  var K: dynamic = pow(2, min(20, k));
  var v: dynamic = cpp_uninitialized();
  var psum: dynamic = cpp_uninitialized();
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var j: dynamic = i;
      var cnt: dynamic = 0;
      while (((j < n) && ((a[i] == a[j]))))
      {
        j += 1;
        cnt += 1;
      }
      v.push_back(a[i]);
      psum.push_back(cnt);
      i = (j - 1);
      i += 1;
    }
  }
  if ((K >= v.size()))
  {
    ans = 0;
  } else
  {
    var sz: dynamic = v.size();
    {
      var i: dynamic = 1;
      while ((i < sz))
      {
        psum[i] += psum[(i - 1)];
        i += 1;
      }
    }
    var i: dynamic = 1;
    var j: dynamic = K;
    ans = (psum[(sz - 1)] - psum[(K - 1)]);
    while ((j < sz))
    {
      var temp: dynamic = (psum[(sz - 1)] - psum[j]);
      temp += psum[(i - 1)];
      ans = min(ans, temp);
      i += 1;
      j += 1;
    }
  }
  write(ans, "\n");
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  solve();
}
