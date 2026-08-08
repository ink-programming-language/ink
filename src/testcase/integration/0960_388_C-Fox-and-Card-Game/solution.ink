// Translated from solution.cpp.

var vec: dynamic;

func main()
{
  var n: dynamic;
  read(n);
  var A = 0;
  var B = 0;
  {
    var i = 0;
    while ((i < n))
    {
      var s: dynamic;
      read(s);
      {
        var j = 0;
        while ((j < (s / 2)))
        {
          var x: dynamic;
          read(x);
          A += x;
          j += 1;
        }
      }
      if ((s % 2))
      {
        var x: dynamic;
        read(x);
        vec.push_back(x);
      }
      {
        var j = 0;
        while ((j < (s / 2)))
        {
          var x: dynamic;
          read(x);
          B += x;
          j += 1;
        }
      }
      i += 1;
    }
  }
  sort(vec.begin(), vec.end());
  reverse(vec.begin(), vec.end());
  {
    var i = 0;
    while ((i < vec.size()))
    {
      if ((i % 2))
      {
        B += vec[i];
      } else
      {
        A += vec[i];
      }
      i += 1;
    }
  }
  write(A, cpp_char(" "), B, "\n");
}
