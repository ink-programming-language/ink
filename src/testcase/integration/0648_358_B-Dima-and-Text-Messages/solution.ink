// Translated from solution.cpp.

func abs(n: dynamic)
{
  return if ((n < 0)) (-n) else n;
}

func gcd(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func main()
{
  var n: dynamic;
  var in_cpp: dynamic;
  var temp: dynamic;
  var line: dynamic;
  while ((scanf("%d", (&n)) == 1))
  {
    temp = "<3";
    {
      var i = 1;
      while ((i <= n))
      {
        read(in_cpp);
        temp += in_cpp;
        temp += "<3";
        i += 1;
      }
    }
    read(line);
    var len = line.size();
    var j = 0;
    {
      var i = 0;
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
