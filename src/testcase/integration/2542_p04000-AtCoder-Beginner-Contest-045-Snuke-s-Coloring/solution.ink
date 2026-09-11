// Translated from solution.cpp.

var MOD: dynamic = cpp_expression("#include <");

var mp: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(h, w, n);
  var ans: dynamic = [];
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      {
        var j: dynamic = 0;
        while ((j <= 2))
        {
          if ((((a - j) <= 0) || (((a - j) + 2) > h)))
          {
            j += 1;
            continue;
          }
          {
            var k: dynamic = 0;
            while ((k <= 2))
            {
              if ((((b - k) <= 0) || (((b - k) + 2) > w)))
              {
                k += 1;
                continue;
              }
              mp[make_pair((a - j), (b - k))] += 1;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  for (var z: dynamic in mp)
  {
    ans[z.second] += 1;
    sum += 1;
  }
  ans[0] = ((((w - 2)) * ((h - 2))) - sum);
  {
    var i: dynamic = 0;
    while ((i <= 9))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
