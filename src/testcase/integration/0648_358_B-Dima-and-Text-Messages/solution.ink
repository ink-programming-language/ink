// Translated from solution.cpp.

func abs(n: dynamic) -> dynamic
{
  return  ((n < 0)) ? (-n) : n;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / gcd(a, b)) * b);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var in_cpp: dynamic = cpp_uninitialized();
  var temp: dynamic = cpp_uninitialized();
  var line: dynamic = cpp_uninitialized();
  while ((scanf("%d", (&n)) == 1))
  {
    temp = "<3";
    {
      var i: dynamic = 1;
      while ((i <= n))
      {
        read(in_cpp);
        temp += in_cpp;
        temp += "<3";
        i += 1;
      }
    }
    read(line);
    var len: dynamic = line.size();
    var j: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < len))
      {
        if ((temp[j] == line[i]))
        {
          j += 1;
        }
        i += 1;
      }
    }
    if ((j == temp.size()))
    {
      printf("yes\n");
    } else
    {
      printf("no\n");
    }
  }
  return 0;
}
