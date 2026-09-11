// Translated from solution.cpp.

var N: dynamic = 100005;

var vmod: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  vmod.push_back(0);
  vmod.push_back(0);
  vmod.push_back(0);
  vmod.push_back(100);
  i = 4;
  while ((i < N))
  {
    vmod.push_back((((vmod[(i - 1)] * 10)) % 210));
    i += 1;
  }
  var n: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  read(n);
  if ((!vmod[n]))
  {
    write(-1, "\n");
  } else
  {
    ans.push_back(1);
    {
      i = 1;
      while ((i < n))
      {
        ans.push_back(0);
        i += 1;
      }
    }
    x = (210 - vmod[i]);
    j = (n - 1);
    while (x)
    {
      ans[j] = (ans[j] + (x % 10));
      x = (x / 10);
      j -= 1;
    }
    {
      j = 0;
      while ((j < ans.size()))
      {
        write(ans[j]);
        j += 1;
      }
    }
  }
  return 0;
}
