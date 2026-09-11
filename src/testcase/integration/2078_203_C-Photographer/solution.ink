// Translated from solution.cpp.

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

class Client
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var idx: dynamic = cpp_uninitialized();
}

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return ((((x.a * A) + (x.b * B))) < (((y.a * A) + (y.b * B))));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(n, d, A, B);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i].a, v[i].b);
      v[i].idx = (i + 1);
      i += 1;
    }
  }
  sort(v.begin(), v.end(), cmp);
  var sum: dynamic = 0;
  var ans: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      sum += (cpp_cast(((v[i].a * A))) + (v[i].b * B));
      if ((sum > d))
      {
        write(ans.size(), "\n");
        {
          var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < ans.size()))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
  return 0;
}
