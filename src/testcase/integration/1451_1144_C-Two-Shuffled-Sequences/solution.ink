// Translated from solution.cpp.

func power(a: dynamic, n: dynamic, m: dynamic)
{
  if ((n == 0))
  {
    return 1;
  }
  var x = power(a, (n / 2), m);
  if (((n % 2) != 0))
  {
    return ((((((((a * x)) % m)) * (x)) % m)) % m);
  } else
  {
    return (((x * x)) % m);
  }
}

var PI = 3.14159265357;

func main()
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var x: dynamic;
  var y: dynamic;
  var m: dynamic;
  var k: dynamic;
  var t: dynamic;
  var sum: dynamic;
  var count: dynamic;
  var cnt: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  var j1: dynamic;
  var z: dynamic;
  var a: dynamic;
  var o: dynamic;
  var temp: dynamic;
  ios.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
  read(n);
  var arr = cpp_array(n);
  var p: dynamic;
  {
    i = 0;
    while ((i < n))
    {
      read(arr[i]);
      p[arr[i]] += 1;
      i += 1;
    }
  }
  var v1: dynamic;
  var v2: dynamic;
  {
    var i1 = p.begin();
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
