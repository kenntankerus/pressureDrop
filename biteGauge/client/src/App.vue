<template>
  <div id="app">
    <nav>
      <ul>
        <li><a href=#>settings</a></li>
        <li><a href=#>login</a></li>
      </ul>
    </nav>
    <dashboard/>
    <bar :chartdata="chartData" :options="chartOptions"/>
  </div>
</template>

<script>
import dashboard from './components/dashboard.vue'
import bar from './components/pressureGauge.vue'

export default {
  name: 'app',
  components: {
    dashboard,
    bar
  }
}
</script>

<style>
#app {
  background: #00baa4;
  color: #2c3e50;
  font-family: 'Avenir', Helvetica, Arial, sans-serif;
  height: 100vh;
  margin: 0px;
  text-align: center;
  -moz-osx-font-smoothing: grayscale;
  -webkit-font-smoothing: antialiased;
  
}
</style>

<!-- 
 #F2EFF1; 
 #6F706E;
 #484A47;
 #8BA59D; 
 #3B8774;
 #00baa4;
  -->