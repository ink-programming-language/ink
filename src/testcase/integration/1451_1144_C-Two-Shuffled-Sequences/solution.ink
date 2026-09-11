// Translated from solution.cpp.

func power(a: dynamic, n: dynamic, m: dynamic) -> dynamic
{
  if ((n == 0))
  {
    return 1;
  }
  var x: dynamic = power(a, (n / 2), m);
  if (((n % 2) != 0))
  {
    return ((((((((a * x)) % m)) * (x)) % m)) % m);
  } else
  {
    return (((x * x)) % m);
  }
}

var PI: dynamic = 3.14159265357;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var sum: dynamic = cpp_uninitialized();
  var count: dynamic = cpp_uninitialized();
  var cnt: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var j1: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var o: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  ios.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  read(n);
  var arr: dynamic = cpp_array(n);
  var p: dynamic = cpp_uninitialized();
  {
    i = 0;
    while ((i < n))
    {
      read(arr[i]);
      p[arr[i]] += 1;
      i += 1;
    }
  }
  var v1: dynamic = cpp_uninitialized();
  var v2: dynamic = cpp_uninitialized();
  {
    var i1: dynamic = p.begin();
    while ((i1 != p.end()))
    {
      x = i1->second;
      if ((x > 2))
      {
        write("NO\n");
        return 0;
      }
      v1.push_back(i1->first);
      x -= 1;
      if ((x != 0))
      {
        v2.push_back(i1->first);
      }
      i1 += 1;
    }
  }
  write("YES\n");
  sort(v1.begin(), v1.end());
  sort(v2.begin(), v2.end(), greater());
  write(v1.size(), "\n");
  {
    i = 0;
    while ((i < v1.size()))
    {
      write(v1[i], " ");
      i += 1;
    }
  }
  write("\n");
  write(v2.size(), "\n");
  {
    i = 0;
    while ((i < v2.size()))
    {
      write(v2[i], " ");
      i += 1;
    }
  }
  return 0;
}
