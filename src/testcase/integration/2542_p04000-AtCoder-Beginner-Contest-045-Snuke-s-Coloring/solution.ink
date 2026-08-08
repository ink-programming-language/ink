// Translated from solution.cpp.

var MOD = cpp_expression("#include <");

var mp: dynamic;

func main()
{
  var h: dynamic;
  var w: dynamic;
  var n: dynamic;
  read(h, w, n);
  var ans = [];
  var sum = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      {
        var j = 0;
        while ((j <= 2))
        {
          if ((((a - j) <= 0) || (((a - j) + 2) > h)))
          {
            j += 1;
            continue;
          }
          {
            var k = 0;
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
  for (var z in mp)
  {
    ans[z.second] += 1;
    sum += 1;
  }
  ans[0] = ((((w - 2)) * ((h - 2))) - sum);
  {
    var i = 0;
    while ((i <= 9))
    {
      write(ans[i], "\n");
      i += 1;
    }
  }
  return 0;
}
