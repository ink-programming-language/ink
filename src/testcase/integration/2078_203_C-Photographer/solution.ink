// Translated from solution.cpp.

var A: dynamic;

var B: dynamic;

class Client
{
  var a: dynamic;
  var b: dynamic;
  var idx: dynamic;
}

func cmp(x: dynamic, y: dynamic)
{
  return ((((x.a * A) + (x.b * B))) < (((y.a * A) + (y.b * B))));
}

func main()
{
  var n: dynamic;
  var d: dynamic;
  read(n, d, A, B);
  {
    var i = 0;
    while ((i < n))
    {
      read(v[i].a, v[i].b);
      v[i].idx = (i + 1);
      i += 1;
    }
  }
  sort(v.begin(), v.end(), cmp);
  var sum = 0;
  var ans: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      sum += (cpp_cast(((v[i].a * A))) + (v[i].b * B));
      if ((sum > d))
      {
        write(ans.size(), "\n");
        {
          var i = 0;
          while ((i < ans.size()))
          {
            write(ans[i], " ");
            i += 1;
          }
        }
        return 0;
      } else
      {
        ans.push_back(v[i].idx);
      }
      i += 1;
    }
  }
  write(ans.size(), "\n");
  {
    var i = 0;
    while ((i < ans.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
