// Translated from solution.cpp.

var B: dynamic = (1e9 + 7);

func Hash(v: dynamic) -> dynamic
{
  var h: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      h = ((h * B) + v[i]);
      i += 1;
    }
  }
  return h;
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var ans: dynamic = 0;
  var n: dynamic = a.size();
  var m: dynamic = b.size();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var S: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_construct(26, 0);
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          if ((i == j))
          {
            S.insert(Hash(v));
          }
          v[(a[j] - cpp_char("a"))] += 1;
          if ((j >= i))
          {
            v[(a[(j - i)] - cpp_char("a"))] -= 1;
            S.insert(Hash(v));
          }
          j += 1;
        }
      }
      S.insert(Hash(v));
      v = vector(26, 0);
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if (((i == j) && S.count(Hash(v))))
          {
            ans = i;
          }
          v[(b[j] - cpp_char("a"))] += 1;
          if ((j >= i))
          {
            v[(b[(j - i)] - cpp_char("a"))] -= 1;
            if (S.count(Hash(v)))
            {
              ans = i;
            }
          }
          j += 1;
        }
      }
      if (S.count(Hash(v)))
      {
        ans = i;
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
