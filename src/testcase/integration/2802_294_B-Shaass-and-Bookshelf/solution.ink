// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(false);
  var ans = INT_MAX;
  var thickOne: dynamic;
  var thickTwo: dynamic;
  var n: dynamic;
  read(n);
  {
    var i = cpp_cast((0));
    while ((i < cpp_cast((n))))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      if ((a == 1))
      {
        thickOne.push_back(b);
      } else
      {
        thickTwo.push_back(b);
      }
      i += 1;
    }
  }
  sort(thickTwo.begin(), thickTwo.end());
  thickTwo.push_back(0);
  reverse(thickTwo.begin(), thickTwo.end());
  sort(thickOne.begin(), thickOne.end());
  thickOne.push_back(0);
  reverse(thickOne.begin(), thickOne.end());
  var widthOne = 0;
  var widthTwo = 0;
  {
    var i = cpp_cast((0));
    while ((i < cpp_cast((thickOne.size()))))
    {
      widthOne += thickOne[i];
      i += 1;
    }
  }
  {
    var i = cpp_cast((0));
    while ((i < cpp_cast((thickTwo.size()))))
    {
      widthTwo += thickTwo[i];
      i += 1;
    }
  }
  {
    var i = cpp_cast((0));
    while ((i < cpp_cast((thickOne.size()))))
    {
      widthOne -= thickOne[i];
      var curr = widthTwo;
      {
        var j = cpp_cast((0));
        while ((j < cpp_cast((thickTwo.size()))))
        {
          curr -= thickTwo[j];
          var abhiThick = (i + (2 * j));
          if (((widthOne + curr) <= abhiThick))
          {
            ans = min(ans, abhiThick);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
