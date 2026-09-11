// Translated from solution.cpp.

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var arr: dynamic = cpp_array(n);
    var v: dynamic = cpp_construct((n + 1), vector());
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(arr[i]);
        v[arr[i]].push_back(i);
        i += 1;
      }
    }
    var ans: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i < n))
      {
        {
          var j: dynamic = (i + 1);
          while ((j < n))
          {
            var y: dynamic = ((upper_bound(v[arr[j]].begin(), v[arr[j]].end(), i) - v[arr[j]].begin()));
            if ((arr[i] == arr[j]))
            {
              y -= 1;
            }
            var x: dynamic = ((upper_bound(v[arr[i]].begin(), v[arr[i]].end(), j) - v[arr[i]].begin()));
            x = (v[arr[i]].size() - x);
            ans += cpp_cast(((cpp_cast(x) * cpp_cast(y))));
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
