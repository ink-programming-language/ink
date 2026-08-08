// Translated from solution.cpp.

var int_cpp = dynamic;

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic;
    var b: dynamic;
    var k: dynamic;
    read(a, b, k);
    var s = cpp_array(k);
    var boy = cpp_array(a);
    var girl = cpp_array(b);
    {
      var i = 0;
      while ((i < a))
      {
        boy[i] = 0;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < b))
      {
        girl[i] = 0;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < k))
      {
        read(s[i].first);
        boy[(s[i].first - 1)] += 1;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < k))
      {
        read(s[i].second);
        girl[(s[i].second - 1)] += 1;
        i += 1;
      }
    }
    var bs = 0;
    {
      var i = 0;
      while ((i < a))
      {
        bs = (bs + ((((boy[i] * ((boy[i] - 1)))) / 2)));
        i += 1;
      }
    }
    var gs = 0;
    {
      var i = 0;
      while ((i < b))
      {
        gs = (gs + ((((girl[i] * ((girl[i] - 1)))) / 2)));
        i += 1;
      }
    }
    var k1 = ((((k) * ((k - 1)))) / 2);
    write((((k1 - gs) - bs)), "\n");
  }
}
