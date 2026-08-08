// Translated from solution.cpp.

var B = (1e9 + 7);

func Hash(v: dynamic)
{
  var h = 0;
  {
    var i = 0;
    while ((i < 26))
    {
      h = ((h * B) + v[i]);
      i += 1;
    }
  }
  return h;
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var ans = 0;
  var n = a.size();
  var m = b.size();
  {
    var i = 1;
    while ((i <= n))
    {
      var S: dynamic;
      var v = cpp_construct(26, 0);
      {
        var j = 0;
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
        var j = 0;
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
